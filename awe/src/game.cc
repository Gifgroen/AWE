#include <stdint.h>

#define internal static
#define global_variable static
#define local_persist static

internal void Render(void *Pixels, int Width, int Height, int XOffset, int YOffset)
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