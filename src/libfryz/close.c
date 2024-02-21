#include "../include/fryz.h"

#include <raylib.h>

extern struct fryz *fryz;

// TODO: put in a utils file or something
void safe_free(void *ptr)
{
    if (ptr == NULL)
        return;
    free(ptr);
    ptr = NULL;
}

void libfryz_close(void)
{
    UnloadMusicStream(fryz->audio.music);
    CloseWindow();
    safe_free(fryz);
}
