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

    fryz->mode = GRAPH_MODE;

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

    InitAudioDevice();
    if (!IsAudioDeviceReady())
    {
        exit_status = 3;
        goto exit_raylib_audio;
    }
    
    char *audio_path = "/home/diego/Projects/Fryz-SF/tests/audio/thunderstruck.wav";
    if (argc > 2)
        audio_path = argv[1];
    fryz->audio.music = LoadMusicStream(audio_path);
    if (!IsMusicReady(fryz->audio.music))
    {
        exit_status = 3;
        goto exit_fryz_music;
    }
    SetMusicVolume(fryz->audio.music, 1.0f);
    PlayMusicStream(fryz->audio.music);
    fryz->audio.paused = false;

    // Success
    return 0;

    // Something went wrong
exit_fryz_music:
    UnloadMusicStream(fryz->audio.music);
exit_raylib_audio:
    CloseAudioDevice();
exit_raylib_window:
    CloseWindow();

    free(fryz);

    return exit_status;
}
