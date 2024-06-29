#include "../include/fryz.h"
#include "../include/graph.h"
#include "../include/math.h"
#include "../include/text.h"

#include <raylib.h>

#include <string.h>
#include <stdbool.h>
#include <math.h>

#define viewport                fryz->graph.viewport
#define left_color              fryz->graph.left_color
#define right_color             fryz->graph.right_color
#define background_color        fryz->graph.background_color
#define border_color            fryz->graph.border_color
#define grid_color              fryz->graph.grid_color
#define text_color              fryz->graph.text_color
#define crosshair_color         fryz->graph.crosshair_color

#define domain          fryz->audio.domain
#define dBSPL_range     fryz->audio.range
#define left            fryz->audio.left
#define right           fryz->audio.right
#define font            fryz->font

extern struct fryz *fryz;

int render_text(struct text_info_t text, Vector2 position,
		enum text_alignment_t alignment)
{
	position = align_text_position(text, position, alignment);

	DrawTextEx(font, text.string, position, text.font_size,
		   text.spacing, text.color);

	return 0;
}

int render_text_rotated(struct text_info_t text, Vector2 position,
			enum text_alignment_t alignment, float rotation)
{
	Vector2 origin = {0};
	// position = align_text_position(text, position, alignment);
	(void)alignment;
	DrawTextPro(font, text.string, position, origin, rotation,
		    text.font_size, text.spacing, text.color);

	return 0;
}

Vector2 align_text_position(struct text_info_t text, Vector2 position,
			    enum text_alignment_t alignment)
{
	Vector2 size = MeasureTextEx(font, text.string, text.font_size,
				     text.spacing);

	// Translate the origin according to the alignment
	switch (alignment)
	{
	case ALIGN_TOP_LEFT:
		break;
	case ALIGN_TOP_CENTER:
		position.x -= size.x / 2;
		break;
	case ALIGN_TOP_RIGHT:
		position.x -= size.x;
		break;
	case ALIGN_RIGHT_CENTER:
		position.x -= size.x;
		position.y -= size.y / 2;
		break;
	case ALIGN_BOTTOM_RIGHT:
		position.x -= size.x;
		position.y -= size.y;
		break;
	case ALIGN_BOTTOM_CENTER:
		position.x -= size.x / 2;
		position.y -= size.y;
		break;
	case ALIGN_BOTTOM_LEFT:
		position.y -= size.y;
		break;
	case ALIGN_LEFT_CENTER:
		position.y -= size.y / 2;
		break;
	case ALIGN_CENTER:
		position.x -= size.x / 2;
		position.y -= size.y / 2;
		break;
	default:
		break;
	}

	return position;
}
