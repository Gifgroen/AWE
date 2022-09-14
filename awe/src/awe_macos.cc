#include <stdio.h>
#include <SDL2/SDL.h>

#define internal static
#define global_variable static
#define local_persist static

global_variable bool Running = true;

#define WindowWidth 1280
#define WindowHeight 1024

global_variable int BitmapWidth = 0;
global_variable int BitmapHeight = 0;

global_variable SDL_Texture *WindowTexture;
global_variable void *Pixels;

internal void Render(int Width, int Height, int XOffset, int YOffset)
{
    int Pitch = Width * sizeof(uint32_t);

    uint8_t *Row = (uint8_t *)Pixels;
    for(int Y = 0; Y < Height; ++Y)
    {
        uint32_t *Pixel = (uint32_t *)Row;
        for(int X = 0; X < Width; ++X)
        {
            uint8_t Blue = Y + YOffset;
            uint8_t Green = 0;
            uint8_t Red = X + XOffset;

            *Pixel++ = (uint32_t)((Red << 16) | (Green << 8) | Blue);
        }
        Row += Pitch;
    }
}

internal void SDLUpdateWindow(SDL_Window *Window, SDL_Renderer *Renderer)
{
    SDL_UpdateTexture(WindowTexture, 0, Pixels, BitmapWidth * sizeof(uint32_t));
    SDL_RenderCopy(Renderer, WindowTexture, 0, 0);
    SDL_RenderPresent(Renderer);
}

internal void ResizeTexture(SDL_Renderer *Renderer, int w, int h) {
    if (WindowTexture) {
        SDL_DestroyTexture(WindowTexture);
        WindowTexture = NULL;
    }

    if (Pixels) {
        free(Pixels);
        Pixels = NULL;
    }

    WindowTexture = SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
    Pixels = (uint32_t *)malloc(w * h * sizeof(uint32_t));
}

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
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                    {
                        BitmapWidth = Event.window.data1;
                        BitmapHeight = Event.window.data2;
                        SDL_Window *Window = SDL_GetWindowFromID(Event.window.windowID);
                        SDL_Renderer *Renderer = SDL_GetRenderer(Window);
                        ResizeTexture(Renderer, BitmapWidth, BitmapHeight);
                    } break;
                    case SDL_WINDOWEVENT_EXPOSED:
                    {
                        SDL_Window *Window = SDL_GetWindowFromID(Event.window.windowID);
                        SDL_Renderer *Renderer = SDL_GetRenderer(Window);
                        SDLUpdateWindow(Window, Renderer);
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

    SDL_GetWindowSize(Window, &BitmapWidth, &BitmapHeight);
    ResizeTexture(Renderer, BitmapWidth, BitmapHeight);

    int XOffset = 0;
    int YOffset = 0;

    while (Running) {
        ProcessInput();
        Render(BitmapWidth, BitmapHeight, XOffset, YOffset);
        SDLUpdateWindow(Window, Renderer);

        ++XOffset;
        XOffset %= BitmapWidth;
        ++YOffset;
        YOffset %= BitmapHeight;
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

    if (Pixels) {
        free(Pixels);
    }

    SDL_Quit();

    return 0;
}
