#include "../include/fryz.h"

#include <raylib.h>

extern struct fryz *fryz;

int libfryz_update(void)
{
    BeginDrawing();

    ClearBackground(RAYWHITE);
    DrawText("Fryz", 190, 200, 20, RED);

    EndDrawing();

    return 0;
}

