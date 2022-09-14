#include <stdio.h>
#include <SDL2/SDL.h>

#include "awe_macos.h"
#include "game.cc"

#define internal static
#define global_variable static
#define local_persist static

global_variable bool Running = true;

#define WindowWidth 1280
#define WindowHeight 1024

SDL_Texture *WindowTexture;

internal void SDLUpdateWindow(offscreen_buffer *global_backbuffer, SDL_Window *Window, SDL_Renderer *Renderer)
{
    SDL_UpdateTexture(WindowTexture, 0, global_backbuffer->Pixels, global_backbuffer->Width * global_backbuffer->BytesPerPixel);
    SDL_RenderCopy(Renderer, WindowTexture, 0, 0);
    SDL_RenderPresent(Renderer);
}

internal void ResizeTexture(offscreen_buffer *global_backbuffer, SDL_Renderer *Renderer) {
    if (WindowTexture) {
        SDL_DestroyTexture(WindowTexture);
        WindowTexture = NULL;
    }

    if (global_backbuffer->Pixels) {
        free(global_backbuffer->Pixels);
        global_backbuffer->Pixels = NULL;
    }

    int w = global_backbuffer->Width;
    int h = global_backbuffer->Height; 
    WindowTexture = SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
    global_backbuffer->Pixels = (uint32_t *)malloc(w * h * global_backbuffer->BytesPerPixel);
}

internal void ProcessInput(offscreen_buffer *global_backbuffer, SDL_Renderer *Renderer) 
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
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                    {
                        global_backbuffer->Width = Event.window.data1;
                        global_backbuffer->Height = Event.window.data2;
                        SDL_Window *Window = SDL_GetWindowFromID(Event.window.windowID);
                        SDL_Renderer *Renderer = SDL_GetRenderer(Window);
                        ResizeTexture(global_backbuffer, Renderer);
                    } break;
                    case SDL_WINDOWEVENT_EXPOSED:
                    {
                        SDL_Window *Window = SDL_GetWindowFromID(Event.window.windowID);
                        SDL_Renderer *Renderer = SDL_GetRenderer(Window);
                        SDLUpdateWindow(global_backbuffer, Window, Renderer);
                    } break;
                }
            } break;
        }
    }
}

int main(int argc, char *argv[]) 
{
    int32_t InitFlags = SDL_INIT_VIDEO;
    if (SDL_Init(InitFlags) != 0) 
    {
        // TODO: Logging!
        printf("Loading SDL failed! %s\n", SDL_GetError());
        return -1;
    }
    
    const char *Title = "AWE Game Engine";
    int Top = SDL_WINDOWPOS_UNDEFINED;
    int Left = SDL_WINDOWPOS_UNDEFINED;
    int32_t CreateWindowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
    SDL_Window *Window = SDL_CreateWindow(Title, Top, Left, WindowWidth, WindowHeight, CreateWindowFlags);
    
    SDL_Renderer *Renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED);
    if (!Renderer) {
        // TODO: Logging!
        return -1;
    }

    int w, h;
    SDL_GetWindowSize(Window, &w, &h);

    offscreen_buffer global_backbuffer = {};
    global_backbuffer.BytesPerPixel = sizeof(uint32_t);
    global_backbuffer.Width = w;
    global_backbuffer.Height = h;

    ResizeTexture(&global_backbuffer, Renderer);

    int XOffset = 0;
    int YOffset = 0;

    while (Running) {
        ProcessInput(&global_backbuffer, Renderer);
        Render(global_backbuffer.Pixels, global_backbuffer.Width, global_backbuffer.Height, XOffset, YOffset);
        SDLUpdateWindow(&global_backbuffer, Window, Renderer);

        ++XOffset;
        XOffset %= global_backbuffer.Width;
        ++YOffset;
        YOffset %= global_backbuffer.Height;
    }

    if (Renderer) {
        SDL_DestroyRenderer(Renderer);
        Renderer = NULL;
    }

    if (Window) {
        SDL_DestroyWindow(Window);
        Window = NULL;
    }

    if (WindowTexture) {
        SDL_DestroyTexture(WindowTexture);
        WindowTexture = NULL;
    }

    if (global_backbuffer.Pixels) {
        free(global_backbuffer.Pixels);
    }

    SDL_Quit();

    return 0;
}
