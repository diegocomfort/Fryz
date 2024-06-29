#ifndef FRYZ_TEXT_H
#define FRYZ_TEXT_H

struct text_info_t
{
	char *string;
	float font_size;
	float spacing;
	Color color;
};

enum text_alignment_t
{
	ALIGN_TOP_LEFT = 0,
	ALIGN_TOP_CENTER,
	ALIGN_TOP_RIGHT,
	ALIGN_RIGHT_CENTER,
	ALIGN_BOTTOM_RIGHT,
	ALIGN_BOTTOM_CENTER,
	ALIGN_BOTTOM_LEFT,
	ALIGN_LEFT_CENTER,
	ALIGN_CENTER,
};

int render_text(struct text_info_t text, Vector2 position,
		enum text_alignment_t alignment);
int render_text_rotated(struct text_info_t text, Vector2 position,
			enum text_alignment_t alignment, float rotation);
Vector2 align_text_position(struct text_info_t text, Vector2 position,
			    enum text_alignment_t alignment);

#endif // FRYZ_TEXT_H
