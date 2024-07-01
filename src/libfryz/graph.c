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

void set_graph_viewport(void)
{
	float width  = GetScreenWidth();
	float height = GetScreenHeight();

	float right_padding = 50;
	float left_padding = 50;
	float top_padding = 50;
	float bottom_padding = 50;

	viewport.x = left_padding;
	viewport.y = top_padding;
	viewport.width = width - right_padding - left_padding;
	viewport.height = height - top_padding - bottom_padding;
}

void render_graph(void)
{
	BeginDrawing(); // Drawing Begin

	ClearBackground(RAYWHITE);
	DrawRectangleRec(viewport, background_color);

	render_horizontal_grid();
	render_vertical_grid();
	render_frequency_responces();
	render_crosshair();
	render_info_at_crosshair();

	// DrawFPS(190, 200);
	// DrawText(TextFormat("%d", GetFPS()), 190, 200, 20, RED);

	EndDrawing();   // Drawing End
}

void render_horizontal_grid(void)
{
	struct range vertical_range;
	vertical_range.min = 0;
	vertical_range.max = viewport.height;

	int tick_count = 20;
	int step = (dBSPL_range.max - dBSPL_range.min) / tick_count;

	struct text_info_t text;
	text.font_size = 20;
	text.spacing = 0;
	text.color = BLACK;
	for (int dBFS = dBSPL_range.max; dBFS >= dBSPL_range.min; dBFS -= step)
	{
		Vector2 begin;
		Vector2 end;
		begin.x = viewport.x;
		begin.y = viewport.y + viewport.height;
		begin.y -= map_range(dBFS, dBSPL_range, vertical_range);
		end.x = viewport.x + viewport.width;
		end.y = begin.y;
		DrawLineV(begin, end, grid_color);

		char dBFS_string[10] = {0};
		snprintf(dBFS_string, 10, "%d", dBFS);
		text.string = dBFS_string;
		enum text_alignment_t alignment = ALIGN_RIGHT_CENTER;
		if (dBFS == dBSPL_range.max)
		{
			alignment = ALIGN_TOP_RIGHT;
		}
		else if (dBFS == dBSPL_range.min)
		{
			alignment = ALIGN_BOTTOM_RIGHT;
		}
		render_text(text, begin, alignment);
	}

	Vector2 axis_title_position;
	axis_title_position.y = viewport.y + viewport.height / 2;
	axis_title_position.x = 0;
	struct text_info_t axis_title;
	axis_title.string = "dBFS";
	axis_title.font_size = 25;
	axis_title.spacing = 0;
	axis_title.color = BLACK;
	render_text_rotated(axis_title, axis_title_position,
			    ALIGN_TOP_LEFT, -90);
}

void render_vertical_grid(void)
{
	// The same ones used at https://squig.link
	float frequencies[] = {2e+1, 3e+1, 4e+1, 5e+1, 6e+1, 8e+1, 1e+2, 1.5e+2,
			       2e+2, 3e+2, 4e+2, 5e+2, 6e+2, 8e+2, 1e+3, 1.5e+3,
			       2e+3, 3e+3, 4e+3, 5e+3, 6e+3, 8e+3, 1e+4, 1.5e+4,
			       2e+4};
	struct text_info_t text;
	text.font_size = 20;
	text.spacing = 0;
	text.color = BLACK;
	for (size_t i = 0; i < LENGTH(frequencies); ++i)
	{
		Vector2 begin;
		Vector2 end;
		begin.x = viewport.x + viewport.width *
			linear_to_log(frequencies[i]);
		begin.y = viewport.y;
		end.x = begin.x;
		end.y = viewport.y + viewport.height;
		DrawLineV(begin, end, grid_color);

		char frequency[10] = {0};
		if (frequencies[i] < 1000)
		{
			snprintf(frequency, 10, "%d", (int) frequencies[i]);
		}
		else
		{
			snprintf(frequency, 10, "%gk",
				 frequencies[i] / 1000);
		}
		text.string = frequency;

		enum text_alignment_t alignment = ALIGN_TOP_CENTER;
		if (frequencies[i] == 20)
		{
			alignment = ALIGN_TOP_LEFT;
		}
		else if (frequencies[i] == 20000)
		{
			alignment = ALIGN_TOP_RIGHT;
		}
		render_text(text, end, alignment);
	}

	Vector2 axis_title_position;
	axis_title_position.y = GetScreenHeight() -
		(GetScreenHeight() - viewport.y - viewport.height) / 2;
	axis_title_position.x = GetScreenWidth() / 2;
	struct text_info_t axis_title;
	axis_title.string = "Hz";
	axis_title.font_size = 30;
	axis_title.spacing = 0;
	axis_title.color = BLACK;
	render_text(axis_title, axis_title_position, ALIGN_TOP_CENTER);
}

void render_frequency_responces(void)
{
	size_t first_index;
	size_t last_index;

	render_and_get_endpoints(&first_index, &last_index);

	Vector2 left_previous_point = project_left_sample(first_index);
	Vector2 right_previous_point = project_right_sample(first_index);

	for (size_t i = first_index + 1; i < last_index; ++i)
	{
		Vector2 left_current_point = project_left_sample(i);
		Vector2 right_current_point = project_right_sample(i);

		DrawLineV(left_previous_point, left_current_point, left_color);
		DrawLineV(right_previous_point, right_current_point,
			  right_color);

		left_previous_point = left_current_point;
		right_previous_point = right_current_point;
	}

	return;
}

void render_and_get_endpoints(size_t *start_index, size_t *end_index)
{
	// size_t first_index = index_of_frequency(domain.min);
	// float frequency_at_first_index = frequency_of_index(first_index);

	// Vector2 position_of_first_left_sample = project_left_sample(first_index); // TODO

	// if (frequency_at_first_index < domain.min)
	// {

	// }
	// else
	// {

	// }
	// return;
	*start_index = index_of_frequency(20);
	*end_index = index_of_frequency(20000);
	return;
}

Vector2 project_left_sample(size_t index)
{
	return project_sample(left.frequency_bins, index);
}

Vector2 project_right_sample(size_t index)
{
	return project_sample(right.frequency_bins, index);
}

Vector2 project_sample(float *frequency_bins, size_t index)
{
	float normalized_sample = frequency_bins[index];
	float frequency = frequency_of_index(index);

	Vector2 position;

	position.x = viewport.x;
	float actual = linear_to_log(frequency);
	float min = linear_to_log(domain.min);
	float max = linear_to_log(domain.max);
	position.x += viewport.width * normalize(actual, min, max);

	// Sample is already normalized to the correct range
	position.y = viewport.y + viewport.height;
	position.y -= normalized_sample * viewport.height;

	return position;
}

Vector2 project_frequency_and_dBSPL(float frequency, float dBSPL)
{
	float normalized_sample = normalize_range(dBSPL, dBSPL_range);

	Vector2 position;

	position.x = viewport.x;
	float actual = linear_to_log(frequency);
	float min = linear_to_log(domain.min);
	float max = linear_to_log(domain.max);
	position.x += viewport.width * normalize(actual, min, max);

	// Sample is already normalized to the correct range
	position.y = viewport.y + viewport.height;
	position.y -= normalized_sample * viewport.height;

	return position;
}

void render_crosshair(void)
{
	Vector2 mouse;
	mouse.x = 1 + GetMouseX();  // For some reason, this bettter represents
				    // the mouse x position
	// I might want to test this with different cursors (TODO)
	mouse.y = GetMouseY();

	if (!CheckCollisionPointRec(mouse, viewport))
		return;

	Vector2 begin;
	Vector2 end;

	// Vertical
	begin.x = mouse.x;
	begin.y = viewport.y;
	end.x = mouse.x;
	end.y = viewport.y + viewport.height;
	DrawLineV(begin, end, crosshair_color);
	// Horizontal
	begin.x = viewport.x;
	begin.y = mouse.y;
	end.x = viewport.x + viewport.width;
	end.y = mouse.y;
	DrawLineV(begin, end, crosshair_color);
}

#define BUFFER_SIZE 32
#define SPACES "                        " // Yes, a bunch of spaces
void render_info_at_crosshair(void)
{
	Vector2 mouse;
	mouse.x = 1 + GetMouseX(); // See render_crosshair()
	mouse.y = GetMouseY();

	if (!CheckCollisionPointRec(mouse, viewport))
	{
		return;
	}

	char info_at_point[BUFFER_SIZE] = {0};
	char left_info_at_frequency[BUFFER_SIZE]  = {0};
	char right_info_at_frequency[BUFFER_SIZE] = {0};
	/* float font_size = 30; */
	/* float font_spacing = 0; */
	/* int horizontal_padding = 0; */
	/* int vertical_padding = 0; */

	float frequency = get_frequency_at_point(mouse);
	float dBSPL = get_dBSPL_at_point(mouse);

	float normalized_dBSPL_at_left_frequency  =
		approximate_normalized_left_dBSPL_at_frequency(frequency);
	float normalized_dBSPL_at_right_frequency =
		approximate_normalized_right_dBSPL_at_frequency(frequency);
	float dBSPL_at_left_frequency  =
		lerp_range(dBSPL_range, normalized_dBSPL_at_left_frequency);
	float dBSPL_at_right_frequency =
		lerp_range(dBSPL_range, normalized_dBSPL_at_right_frequency);;

	snprintf(info_at_point,            BUFFER_SIZE,
		 "(%06d Hz, %6g dBFS)", (int)frequency, dBSPL);
	// Yes, its hacky as fuck, but printf just does not have the right
	// format specifier for me
	snprintf(left_info_at_frequency,  BUFFER_SIZE,
		 "Left:  %04d.%03d dBFS%s",
		 (int)dBSPL_at_left_frequency,
		 abs((int)(dBSPL_at_left_frequency * 1000) % 1000), SPACES);
	snprintf(right_info_at_frequency,  BUFFER_SIZE,
		 "Right: %04d.%03d dBFS%s",
		 (int)dBSPL_at_right_frequency,
		 abs((int)(dBSPL_at_right_frequency * 1000) % 1000), SPACES);

	struct text_info_t text_info_at_point;
	text_info_at_point.string = info_at_point;
	text_info_at_point.font_size = 25;
	text_info_at_point.spacing = 0;
	text_info_at_point.color = BLACK;
	render_text(text_info_at_point, mouse, ALIGN_BOTTOM_LEFT);

	// Draw circles on frequency responce graphs
	float radius = 7;
	Vector2 left_position;
	Vector2 right_position;

	left_position.x = mouse.x;
	left_position.y = viewport.y + viewport.height;
	left_position.y -= viewport.height * normalized_dBSPL_at_left_frequency;
	DrawCircleV(left_position, radius, left_color);
	struct text_info_t left_info;
	left_info.string = left_info_at_frequency;
	left_info.font_size = 25;
	left_info.spacing = 1;
	left_info.color = RED;
	Vector2 left_info_position;
	left_info_position.x = viewport.x + viewport.width;
	left_info_position.y = viewport.y + viewport.height / 6;
	render_text(left_info, left_info_position, ALIGN_RIGHT_CENTER);

	right_position.x = mouse.x;
	right_position.y = viewport.y + viewport.height;
	right_position.y -= viewport.height *
		normalized_dBSPL_at_right_frequency;
	DrawCircleV(right_position, radius, right_color);
	struct text_info_t right_info;
	right_info.string = right_info_at_frequency;
	right_info.font_size = 25;
	right_info.spacing = 1;
	right_info.color = BLUE;
	Vector2 right_info_position;
	right_info_position.x = viewport.x + viewport.width;
	right_info_position.y = viewport.y + viewport.height / 6 + 30;
	render_text(right_info, right_info_position, ALIGN_RIGHT_CENTER);
}
#undef SPACES
#undef BUFFER_SIZE
