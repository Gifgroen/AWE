#include <stdint.h>

#include "game.h"

static int XOffset = 0;
static int YOffset = 0;

void Render(offscreen_buffer *Buffer, game_input *Input)
{
    game_button_state *MoveUp = &Input->Controllers[0].MoveUp;
    if (MoveUp->HalfTransitionCount > 0) {
        printf("UP HalfTransitionCount = %d\n", MoveUp->HalfTransitionCount);
    }
    if (MoveUp->EndedDown) 
    {
        YOffset++;
    }
 
    game_button_state *MoveLeft = &Input->Controllers[0].MoveLeft;
    if (MoveLeft->HalfTransitionCount > 0) {
        printf("LEFT HalfTransitionCount = %d\n", MoveLeft->HalfTransitionCount);
    }
    if (MoveLeft->EndedDown) 
    {
        XOffset++;
    }

    int Pitch = Buffer->Width * Buffer->BytesPerPixel;

    uint8_t *Row = (uint8_t *)Buffer->Pixels;
    for(int Y = 0; Y < Buffer->Height; ++Y)
    {
        uint32_t *Pixel = (uint32_t *)Row;
        for(int X = 0; X < Buffer->Width; ++X)
        {
            uint8_t Blue = Y + YOffset;
            uint8_t Green = 0;
            uint8_t Red = X + XOffset;

            *Pixel++ = (uint32_t)((Red << 16) | (Green << 8) | Blue);
        }
        Row += Pitch;
    }
}