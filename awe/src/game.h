#ifndef GAME_H
#define GAME_H

struct offscreen_buffer 
{
    int Width;
    int Height;
    int BytesPerPixel;
    int Pitch;
    void *Pixels;
};

#endif /* GAME_H */
