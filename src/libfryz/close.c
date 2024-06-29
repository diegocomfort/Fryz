#include "../include/fryz.h"
#include "../include/audio.h"

#include <raylib.h>

extern struct fryz *fryz;

void libfryz_close(void)
{
	DetachAudioStreamProcessor(fryz->audio.music.stream,
				   raylib_waveform_capture_callback);
	UnloadMusicStream(fryz->audio.music);
	UnloadFont(fryz->font);
	CloseWindow();
	close_fft_data_sets();
	free(fryz);
}
