#include <stdio.h>
#include <sys/mman.h>
#include <SDL2/SDL.h>

#include "awe_macos.h"  // MacOS Platform Layer
#include "game.cc"      // Game "service" to be used by platform

#define internal static
#define global_variable static
#define local_persist static

global_variable bool Running = true;

SDL_Texture *WindowTexture;

global_variable int MAX_CONTROLLERS = 4;

internal int SDLSetupGameControllers(int ControllerCount, SDL_GameController *Result[]) {
    int ConnectedControllerCount = 0;
    for(int controllerIndex = 0; controllerIndex < ControllerCount; ++controllerIndex) {
        if (!SDL_IsGameController(controllerIndex))
        {
            printf("Not a game controller!\n");
            continue;
        }
        if (controllerIndex >= MAX_CONTROLLERS)
        {
            printf("Max controller count reached!\n");
            break;
        }
        Result[controllerIndex] = SDL_GameControllerOpen(controllerIndex);
        if(Result[controllerIndex] == NULL)
        {
            printf("Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError());
            continue;
        }
        ++ConnectedControllerCount;
    }
    return ConnectedControllerCount;
}

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

internal void ProcessInput(offscreen_buffer *buffer, SDL_Renderer *Renderer) 
{
    SDL_Event Event;

    while(SDL_PollEvent(&Event)) 
    {
        switch(Event.type)
        {
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
    
    // Setup Controller subsystem!
    int ControllerCount = SDL_NumJoysticks();

    // TODO: assert that ControllerCount <= MAX_CONTROLLERS
    SDL_GameController *ControllerHandles[MAX_CONTROLLERS];
    int ConnectedControllerCount = SDLSetupGameControllers(ControllerCount, ControllerHandles);

    // Setup Window
    const char *Title = "AWE Game Engine";
    int Top = SDL_WINDOWPOS_UNDEFINED;
    int Left = SDL_WINDOWPOS_UNDEFINED;
    int WindowWidth = 1280;
    int WindowHeight = 1024;
    int32_t CreateWindowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
    SDL_Window *Window = SDL_CreateWindow(Title, Top, Left, WindowWidth, WindowHeight, CreateWindowFlags);
    
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

    int XOffset = 0;
    int YOffset = 0;

    while (Running) 
    {
        ProcessInput(&offscreen_buffer, Renderer);
        for (int ControllerIndex = 0; ControllerIndex < ConnectedControllerCount; ++ControllerIndex)
        {
            SDL_GameController *Controller = ControllerHandles[ControllerIndex];
            if(Controller != NULL && SDL_GameControllerGetAttached(Controller))
            {
                // TODO: process input from this connected GameController

                if (SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_DPAD_UP))
                {
                    ++YOffset;
                }
                if (SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN))
                {
                    --YOffset;
                }

                if (SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT))
                {
                    ++XOffset;
                }
                if (SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
                {
                    --XOffset;
                }
            }
            else
            {
                // TODO: Logging!
            }
        }

        Render(&offscreen_buffer, XOffset, YOffset);
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
