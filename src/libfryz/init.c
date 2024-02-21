#include "../include/fryz.h"

#include <raylib.h>

#include <stdlib.h>

#ifdef FRYZ_DEBUG               
    #define FRYZ_LOG_LEVEL LOG_ALL
#else                   
    #define FRYZ_LOG_LEVEL LOG_ERROR
#endif

struct fryz *fryz;

int libfryz_init(int argc, char **argv)
{
    fryz = malloc(sizeof(*fryz));
    if (fryz == NULL)
        return 1;

    /* int error; */
    int exit_status;

    SetTraceLogLevel(FRYZ_LOG_LEVEL);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800 + argc*0, 450, argv[0]);
    if (!IsWindowReady())
    {
        exit_status = 2;
        goto exit_raylib_window;
    }

    return 0;

exit_raylib_window:
    CloseWindow();

    free(fryz);

    return exit_status;
}
