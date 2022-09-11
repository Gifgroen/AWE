#include <stdio.h>
#include <SDL2/SDL.h>

#define internal static
#define global_variable static
#define local_persist static

global_variable bool Running = true;
global_variable SDL_Window *Window;
global_variable SDL_Surface *ScreenSurface;

global_variable int WindowWidth = 1280;
global_variable int WindowHeight = 1024;

internal void ProcessInput(void) 
{
    SDL_Event Event;

    while(SDL_PollEvent(&Event)) 
    {
        switch(Event.type)
        {
            case SDL_QUIT: 
            {
                Running = false;
                printf("Processed: SDL_QUIT!\n");
            } break;
            case SDL_WINDOWEVENT:
            {
                switch(Event.window.event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                    {
                        printf("Processed: SDL_WINDOWEVENT_RESIZED (%d, %d)!\n", Event.window.data1, Event.window.data2);
                    } break;
                }
            } break;
        }
    }
}

internal void Render(void) 
{    
    SDL_FillRect(ScreenSurface, NULL, 0xFF00FF);
    SDL_UpdateWindowSurface(Window);
}

int main(int argc, char *argv[]) 
{
    int32_t InitFlags = SDL_INIT_VIDEO;
    if (SDL_Init(InitFlags) != 0) 
    {
        printf("Loading SDL failed! %s\n", SDL_GetError());
        return -1;
    }
    
    const char *Title = "AWE Game Engine";
    int Top = SDL_WINDOWPOS_UNDEFINED;
    int Left = SDL_WINDOWPOS_UNDEFINED;
    int32_t CreateWindowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
    Window = SDL_CreateWindow(Title, Top, Left, WindowWidth, WindowHeight, CreateWindowFlags);
    ScreenSurface = SDL_GetWindowSurface(Window);

    while (Running) {
        ProcessInput();
        Render();
    }

    SDL_FreeSurface(ScreenSurface);
    ScreenSurface = NULL;

    SDL_DestroyWindow(Window);
    Window = NULL;

    SDL_Quit();

    return 0;
}
