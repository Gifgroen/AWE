#ifndef GAME_H
#define GAME_H

#define MAX_CONTROLLERS 4

struct offscreen_buffer 
{
    int Width;
    int Height;
    int BytesPerPixel;
    int Pitch;
    void *Pixels;
};

struct game_button_state {
    bool EndedDown;
    int HalfTransitionCount;
};

struct game_controller_input
{
    bool IsConnected;
    bool IsAnalog;
    bool StickAverageX;
    bool StickAverageY;

    union
    {
        game_button_state Buttons[12];
        struct
        {
            game_button_state MoveUp;
            game_button_state MoveDown;
            game_button_state MoveLeft;
            game_button_state MoveRight;

            game_button_state ActionUp;
            game_button_state ActionDown;
            game_button_state ActionLeft;
            game_button_state ActionRight;

            game_button_state LeftShoulder;
            game_button_state RightShoulder;

            game_button_state Back;
            game_button_state Start;

            // NOTE(casey): All buttons must be added above this line

            game_button_state Terminator;
        };
    };
};

struct game_input
{
    game_controller_input KeyboardController[1];
	game_controller_input Controllers[MAX_CONTROLLERS];
};

#endif /* GAME_H */
