#include "../include/fryz.h"
#include "../include/audio.h"

#include <raylib.h>

#include <stdlib.h>

#ifdef FRYZ_DEBUG               
    #define FRYZ_LOG_LEVEL LOG_ALL
#else                   
    #define FRYZ_LOG_LEVEL LOG_WARNING
#endif

struct fryz *fryz;

int libfryz_init(int argc, char **argv)
{
    fryz = calloc(1, sizeof(*fryz));
    if (fryz == NULL)
    {
        TraceLog(LOG_ERROR, "Kill some chrome tabs!");
        return 1;
    }

    fryz->mode = GRAPH_MODE;

    int error;
    int exit_status;

    SetTraceLogLevel(FRYZ_LOG_LEVEL);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800 + argc*0, 450, argv[0]);
    if (!IsWindowReady())
    {
        TraceLog(LOG_ERROR, "Failed to initialize a window");
        exit_status = 2;
        goto exit_raylib_window;
    }

    InitAudioDevice();
    if (!IsAudioDeviceReady())
    {
        TraceLog(LOG_ERROR, "Failed to initialize audio device");
        exit_status = 3;
        goto exit_raylib_audio;
    }
    
    char *audio_path = "/home/diego/Projects/Fryz-SF/tests/audio/thunderstruck.wav";
    if (argc > 1)
        audio_path = argv[1];
    fryz->audio.music = LoadMusicStream(audio_path);
    if (!IsMusicReady(fryz->audio.music))
    {
        TraceLog(LOG_ERROR, "Failed to load music: %s", audio_path);
        exit_status = 3;
        goto exit_fryz_music;
    }
    SetMusicVolume(fryz->audio.music, 1.0f);
    PlayMusicStream(fryz->audio.music);
    fryz->audio.paused = false;
    AttachAudioStreamProcessor(fryz->audio.music.stream, raylib_waveform_capture_callback);
    set_max_sample_size();

    // Set default options (maybe set with arguments later)
    fryz->audio.left.smoothing_factor = 0;  // Default to no smoothing
    fryz->audio.right.smoothing_factor = 0;
    fryz->audio.domain.min = 20;
    fryz->audio.domain.max = 20000;
    fryz->audio.range.max = 0;
    fryz->audio.range.min = -125;
    error = set_fft_size_and_init_data_sets(1 << 12);
    if (error)
    {
        TraceLog(LOG_ERROR, "Failed to iniliaze the fft datasets");
        exit_status = 4;
        goto exit_fryz_fft_data;
    }

    // Success
    return 0;

    // Something went wrong
exit_fryz_fft_data:
    close_fft_data_sets();
exit_fryz_music:
    UnloadMusicStream(fryz->audio.music);
exit_raylib_audio:
    CloseAudioDevice();
exit_raylib_window:
    CloseWindow();

    free(fryz);

    return exit_status;
}
