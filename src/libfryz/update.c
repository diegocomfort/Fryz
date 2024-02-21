#include "../include/fryz.h"

#include <raylib.h>

extern struct fryz *fryz;

void handle_key_presses(void);

int libfryz_update(void)
{
    UpdateMusicStream(fryz->audio.music);

    handle_key_presses();
    
    BeginDrawing();

    ClearBackground(RAYWHITE);
    DrawText("Fryz", 190, 200, 20, RED);

    EndDrawing();

    return 0;
}

void handle_key_presses(void)
{
    int key;
    while ((key = GetKeyPressed()))
    {
	switch (key)
	{
	case KEY_SPACE:
	{
	    if (fryz->audio.paused)
		ResumeMusicStream(fryz->audio.music);
	    else
		PauseMusicStream(fryz->audio.music);
	    fryz->audio.paused = !fryz->audio.paused;
	}
	default: break;
	}
    }
}

