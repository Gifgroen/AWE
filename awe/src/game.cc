#include <stdint.h>

#define internal static
#define global_variable static
#define local_persist static

internal void Render(offscreen_buffer *buffer, int XOffset, int YOffset)
{
    int Pitch = buffer->Width * buffer->BytesPerPixel;

    uint8_t *Row = (uint8_t *)buffer->Pixels;
    for(int Y = 0; Y < buffer->Height; ++Y)
    {
        uint32_t *Pixel = (uint32_t *)Row;
        for(int X = 0; X < buffer->Width; ++X)
        {
            uint8_t Blue = Y + YOffset;
            uint8_t Green = 0;
            uint8_t Red = X + XOffset;

            *Pixel++ = (uint32_t)((Red << 16) | (Green << 8) | Blue);
        }
        Row += Pitch;
    }
}