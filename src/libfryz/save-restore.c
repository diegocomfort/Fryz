#include "../include/fryz.h"
#include "../include/audio.h"

extern struct fryz *fryz;

struct fryz *libfryz_save(void)
{
	// Not dettaching and reattaching the processor (callback) is what
	// was causing the stack corruption. Becase when libfryz is
	// reloaded, the callback will be moved to a new position in
	// memory, so some random chunk of memory was being interpreted as
	// the callback, leading to illegal instructions and all sorts of
	// fun errors. The funny part is that gdb always showed the errors
	// at the callback, but I couldn't figure out why. Now that I now,
	// I remember seeing the dettaching and reattaching in Tsoding's
	// musializer code, which I had never thought of once I started
	// working on this.
	DetachAudioStreamProcessor(fryz->audio.music.stream,
				   raylib_waveform_capture_callback);
	return fryz;
}

void libfryz_restore(struct fryz *state)
{
	fryz = state;
	AttachAudioStreamProcessor(fryz->audio.music.stream,
				   raylib_waveform_capture_callback);
	fryz->audio.left.smoothing_factor = 0;
	TraceLog(LOG_INFO, "Succesfully reloaded libfryz");
}
