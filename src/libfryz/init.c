#include "../include/fryz.h"
#include "../include/audio.h"
#include "../include/graph.h"

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
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
	SetTargetFPS(60);
	InitWindow(1440, 810, argv[0]);
	if (!IsWindowReady())
	{
		TraceLog(LOG_ERROR, "Failed to initialize a window");
		exit_status = 2;
		goto exit_raylib_window;
	}
	SetExitKey(KEY_NULL);

	InitAudioDevice();
	if (!IsAudioDeviceReady())
	{
		TraceLog(LOG_ERROR, "Failed to initialize audio device");
		exit_status = 3;
		goto exit_raylib_audio;
	}

	char *audio_path = DEFAULT_AUDIO_PATH;
	if (argc > 1)
		audio_path = argv[1];
	fryz->audio.music = LoadMusicStream(audio_path);
	if (!IsMusicReady(fryz->audio.music))
	{
		TraceLog(LOG_ERROR, "Failed to load music: %s", audio_path);
		exit_status = 3;
		goto exit_fryz_music;
	}
	fryz->audio.volume = 1;
	fryz->audio.muted = false;
	SetMusicVolume(fryz->audio.music, fryz->audio.volume);
	fryz->audio.paused = false;
	if (argc > 2)
	{
		float time = atof(argv[2]);
		SeekMusicStream(fryz->audio.music, time);
	}
	PlayMusicStream(fryz->audio.music);
	AttachAudioStreamProcessor(fryz->audio.music.stream,
				   raylib_waveform_capture_callback);

	// Set default options (maybe set with arguments later)
	fryz->audio.left.smoothing_factor = 0;  // Default to no smoothing
	fryz->audio.right.smoothing_factor = 0;
	fryz->audio.domain.min = 20;
	fryz->audio.domain.max = 20000;
	fryz->audio.range.max = 0;
	fryz->audio.range.min = -120;
	error = set_fft_size_and_init_data_sets(1 << 12);
	if (error)
	{
		TraceLog(LOG_ERROR, "Failed to iniliaze the fft datasets");
		exit_status = 4;
		goto exit_fryz_fft_data;
	}

	// Set default graph settings
	set_graph_viewport();
	fryz->graph.left_color              = RED;
	fryz->graph.right_color             = BLUE;
	fryz->graph.background_color        = LIGHTGRAY;
	fryz->graph.border_color            = DARKGRAY;
	fryz->graph.grid_color              = WHITE;
	fryz->graph.text_color              = WHITE;
	fryz->graph.crosshair_color         = BLACK;

	// Load font
	fryz->font = LoadFont(FRYZ_FONT_PATH);
	if (!IsFontReady(fryz->font))
	{
		TraceLog(LOG_ERROR, "Failed to load font %s", FRYZ_FONT_PATH);
		exit_status = 5;
		goto exit_fryz_font;
	}
	SetTextureFilter(fryz->font.texture, TEXTURE_FILTER_BILINEAR);

	// Success
	return 0;

	// Something went wrong
exit_fryz_font:
	UnloadFont(fryz->font);
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
