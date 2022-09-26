#ifndef AWE_MACOS_H
#define AWE_MACOS_H

struct window_dimension {
    int Width;
    int Height;
};

struct offscreen_buffer {
    int Width;
    int Height;
    int BytesPerPixel;
    int Pitch;
    void *Pixels;
};

#endif /* AWE_MACOS_H */
