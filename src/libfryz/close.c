#include "../include/fryz.h"

#include <raylib.h>

extern struct fryz *fryz;

void libfryz_close(void)
{
    UnloadMusicStream(fryz->audio.music);
    CloseWindow();
    free(fryz);
}
