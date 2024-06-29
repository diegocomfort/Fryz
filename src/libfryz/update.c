#include "../include/fryz.h"
#include "../include/audio.h"
#include "../include/graph.h"

#include <raylib.h>

extern struct fryz *fryz;

void handle_key_presses(void);

int libfryz_update(void)
{
	UpdateMusicStream(fryz->audio.music);
	process_audio();

	handle_key_presses();

	set_graph_viewport();
	render_graph();

	return 0;
}

void handle_key_presses(void)
{
	int key;
	while ((key = GetKeyPressed()))
	{
		switch (key)
		{
		case KEY_M:
			toggle_mute();
			break;
		case KEY_SPACE:
		case KEY_SEMICOLON:
			toggle_pause();
			break;
		case KEY_H:
			skip(-10.0);
			break;
		case KEY_L:
			skip(10.0);
			break;
		default: continue;
		}
	}
}
