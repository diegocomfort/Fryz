#ifndef FRYZ_GRAPH_H
#define FRYZ_GRAPH_H

#include <raylib.h>

void set_graph_viewport(void);
void render_graph(void);

void render_horizontal_grid(void);
void render_vertical_grid(void);
void render_frequency_responces(void);
void render_crosshair(void);
void render_info_at_crosshair(void);

void render_and_get_endpoints(size_t *start_index, size_t *end_index);
Vector2 project_left_sample(size_t index);
Vector2 project_right_sample(size_t index);
Vector2 project_sample(float *frequency_bins, size_t index);
Vector2 project_frequency_and_dBSPL(float frequency, float dBSPL);

#endif // FRYZ_GRAPH_H
