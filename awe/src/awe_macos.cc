#include <stdio.h>
#include <sys/mman.h>
#include <SDL2/SDL.h>

#include "awe_macos.h"  // MacOS Platform Layer
#include "game.cc"      // Game "service" to be used by platform

#define internal static
#define global_variable static
#define local_persist static

global_variable bool Running = true;

#define ArrayCount(Array) (sizeof(Array) / sizeof(*(Array)))

SDL_Texture *WindowTexture;

// Rendering
internal void SDLUpdateWindow(offscreen_buffer *buffer, SDL_Texture *WindowTexture, SDL_Renderer *Renderer)
{
    SDL_UpdateTexture(WindowTexture, 0, buffer->Pixels, buffer->Width * buffer->BytesPerPixel);
    SDL_RenderCopy(Renderer, WindowTexture, 0, 0);
    SDL_RenderPresent(Renderer);
}

internal window_dimension SDLGetWindowDimension(SDL_Window *Window) 
{
    window_dimension Result = {};
    SDL_GetWindowSize(Window, &Result.Width, &Result.Height);
    return Result;
}

internal void SDLResizeTexture(offscreen_buffer *buffer, window_dimension NewDimensions, SDL_Renderer *Renderer) 
{
    if (WindowTexture) 
    {
        SDL_DestroyTexture(WindowTexture);
        WindowTexture = NULL;
    }

    if (buffer->Pixels) 
    {
        munmap(buffer->Pixels, buffer->Width * buffer->Height * buffer->BytesPerPixel);
    }

    int Width = NewDimensions.Width;
    int Height = NewDimensions.Height; 
    WindowTexture = SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, Width, Height);

    buffer->Pixels = mmap(0, Width * Height * buffer->BytesPerPixel, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    buffer->Width = Width;
    buffer->Height = Height;
    buffer->Pitch = buffer->Width * buffer->BytesPerPixel;
}

// GamePad/Joystick Input
internal int SDLSetupGameControllers(SDL_GameController *Result[]) {
    int ControllerCount = SDL_NumJoysticks();
    int ConnectedControllerCount = 0;
    for(int ControllerIndex = 1; ControllerIndex < ControllerCount; ++ControllerIndex) {
        if (!SDL_IsGameController(ControllerIndex))
        {
            printf("Not a game controller!\n");
            continue;
        }
        if (ControllerIndex >= MAX_CONTROLLERS)
        {
            printf("Max controller count reached!\n");
            break;
        }
        Result[ControllerIndex] = SDL_GameControllerOpen(ControllerIndex);
        if(Result[ControllerIndex] == NULL)
        {
            printf("Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError());
            continue;
        }
        ++ConnectedControllerCount;
    }
    return ConnectedControllerCount;
}

internal void SDLProcessGameControllerButton(game_button_state *OldState, game_button_state *NewState, bool Value)
{
    NewState->EndedDown = Value;
    NewState->HalfTransitionCount += (OldState->EndedDown == NewState->EndedDown) ? 0 : 1;
}

internal void SDLProcessKeyPress(game_button_state *NewState, bool IsDown)
{
    if (NewState->EndedDown == IsDown) {
        return;
    }
    NewState->EndedDown = IsDown;
    ++NewState->HalfTransitionCount;

}

// Keyboard/Mouse Input 
internal void ProcessInput(offscreen_buffer *buffer, SDL_Renderer *Renderer, game_controller_input *KeyboardController) 
{
    SDL_Event Event;

    while(SDL_PollEvent(&Event)) 
    {
        switch(Event.type)
        {
            case SDL_KEYDOWN:
            case SDL_KEYUP: 
            {
                SDL_Keycode KeyCode = Event.key.keysym.sym;
                bool IsDown = Event.key.state == SDL_PRESSED;
                printf("[DEBUG] ProcessInput: KeyCode = %d, IsDown = %d.\n", KeyCode, IsDown);

                if (Event.key.repeat == 0)
                {
                    if(KeyCode == SDLK_UP || KeyCode == SDLK_w)
                    {
                        SDLProcessKeyPress(&KeyboardController->MoveUp, IsDown);
                    }
                    else if(KeyCode == SDLK_LEFT || KeyCode == SDLK_a)
                    {
                        SDLProcessKeyPress(&KeyboardController->MoveLeft, IsDown);
                    }
                    else if(KeyCode == SDLK_DOWN || KeyCode == SDLK_s)
                    {
                        SDLProcessKeyPress(&KeyboardController->MoveDown, IsDown);
                    }
                    else if(KeyCode == SDLK_RIGHT || KeyCode == SDLK_d)
                    {
                        SDLProcessKeyPress(&KeyboardController->MoveRight, IsDown);
                    }
                    else if(KeyCode == SDLK_ESCAPE)
                    {
                        printf("ESCAPE: ");
                        if(IsDown)
                        {
                            printf("IsDown\n");
                        }
                    }
                }
            } break;
            case SDL_QUIT: 
            {
                Running = false;
            } break;
            case SDL_WINDOWEVENT:
            {
                switch(Event.window.event)
                {
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                    {
                        window_dimension dim = {};
                        dim.Width = Event.window.data1;
                        dim.Height = Event.window.data2;
                        SDL_Window *Window = SDL_GetWindowFromID(Event.window.windowID);
                        SDL_Renderer *Renderer = SDL_GetRenderer(Window);
                        SDLResizeTexture(buffer, dim, Renderer);
                    } break;
                    case SDL_WINDOWEVENT_EXPOSED:
                    {
                        SDL_Window *Window = SDL_GetWindowFromID(Event.window.windowID);
                        SDL_Renderer *Renderer = SDL_GetRenderer(Window);
                        SDLUpdateWindow(buffer, WindowTexture, Renderer);
                    } break;
                }
            } break;
        }
    }
}

int main(int argc, char *argv[]) 
{
    int32_t SubsystemFlags = SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER;
    if (SDL_Init(SubsystemFlags) != 0)
    {
        // TODO: Logging!
        printf("Loading SDL failed! %s\n", SDL_GetError());
        return -1;
    }

    game_input Input[2] = {};
    game_input *OldInput = &Input[0];
    game_input *NewInput = &Input[1];

    SDL_GameController *ControllerHandles[MAX_CONTROLLERS];
    int ConnectedControllerCount = SDLSetupGameControllers(ControllerHandles);

    // Setup Window
    const char *Title = "AWE Game Engine";
    int Top = SDL_WINDOWPOS_UNDEFINED;
    int Left = SDL_WINDOWPOS_UNDEFINED;
    int WindowWidth = 1280;
    int WindowHeight = 1024;
    int32_t CreateWindowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
    SDL_Window *Window = SDL_CreateWindow(Title, Top, Left, WindowWidth, WindowHeight, CreateWindowFlags);
    if (!Window) 
    {
        // TODO: Logging!
        return -1;
    }
    
    SDL_Renderer *Renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED);
    if (!Renderer) 
    {
        // TODO: Logging!
        return -1;
    }

    // the Width and Height of offscreen_buffer are set by passed window_dimensions
    offscreen_buffer offscreen_buffer = {};
    offscreen_buffer.BytesPerPixel = sizeof(uint32_t);

    window_dimension newDim = SDLGetWindowDimension(Window);
    offscreen_buffer.Pitch = offscreen_buffer.BytesPerPixel * newDim.Width;
    SDLResizeTexture(&offscreen_buffer, newDim, Renderer);

    while (Running)
    {
        game_controller_input *OldKeyboardController = &OldInput->Controllers[0];
        game_controller_input *NewKeyboardController = &NewInput->Controllers[0];
        *NewKeyboardController = {0};
        for(int ButtonIndex = 0; ButtonIndex < ArrayCount(NewKeyboardController->Buttons); ++ButtonIndex) {
            NewKeyboardController->Buttons[ButtonIndex].EndedDown =
            OldKeyboardController->Buttons[ButtonIndex].EndedDown;
        }

        ProcessInput(&offscreen_buffer, Renderer, &NewInput->Controllers[Keyboard]);

        for (int ControllerIndex = 0; ControllerIndex < ConnectedControllerCount; ++ControllerIndex)
        {
            game_controller_input *OldController = &OldInput->Controllers[ControllerIndex];
            game_controller_input *NewController = &NewInput->Controllers[ControllerIndex];

            SDL_GameController *Controller = ControllerHandles[ControllerIndex];
            if(Controller != NULL && SDL_GameControllerGetAttached(Controller))
            {
                // TODO: process input from this connected GameController

                SDLProcessGameControllerButton(&(OldController->MoveUp), &(NewController->MoveUp), SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_DPAD_UP));
                SDLProcessGameControllerButton(&(OldController->MoveLeft), &(NewController->MoveLeft), SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT));
                SDLProcessGameControllerButton(&(OldController->MoveDown), &(NewController->MoveDown), SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN));
                SDLProcessGameControllerButton(&(OldController->MoveRight), &(NewController->MoveRight), SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT));

                // TODO: process Joystick and Moar Buttons
            }
            else
            {
                // TODO: Logging!
            }
        }

        Render(&offscreen_buffer, NewInput);

        game_input *Temp = NewInput;
        NewInput = OldInput;
        OldInput = Temp;

        SDLUpdateWindow(&offscreen_buffer, WindowTexture, Renderer);
    }

    for (int i = 0; i < ConnectedControllerCount; ++i)
    {
        SDL_GameController *controller = ControllerHandles[i];
        if (!controller)
        {
            printf("Could not close gamecontroller %i: %s\n", i, SDL_GetError());
            continue;
        }
        SDL_GameControllerClose(controller);
    }

    if (Renderer) 
    {
        SDL_DestroyRenderer(Renderer);
        Renderer = NULL;
    }

    if (Window) 
    {
        SDL_DestroyWindow(Window);
        Window = NULL;
    }

    if (WindowTexture) 
    {
        SDL_DestroyTexture(WindowTexture);
        WindowTexture = NULL;
    }

    if (offscreen_buffer.Pixels) 
    {
        munmap(offscreen_buffer.Pixels, offscreen_buffer.Width * offscreen_buffer.Height * offscreen_buffer.BytesPerPixel);
    }

    SDL_Quit();

    return 0;
}
