#include "../include/fryz.h"
#include "../include/audio.h"

#include <raylib.h>

extern struct fryz *fryz;

void handle_key_presses(void);

int libfryz_update(void)
{
    UpdateMusicStream(fryz->audio.music);
    /* process_audio(); */

    handle_key_presses();
    
    BeginDrawing(); // Drawing

    ClearBackground(RAYWHITE);
    DrawText("Fryz", 190, 200, 20, RED);

    EndDrawing();   // Drawing

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
	break;
	default: continue;
	}
    }
}

