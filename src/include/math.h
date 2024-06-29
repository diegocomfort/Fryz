#ifndef FRYZ_MATH_H
#define FRYZ_MATH_H

#include <raylib.h>

#include <stdlib.h>

struct range
{
	float min;
	float max;
};

#define LENGTH(arr) (sizeof((arr)) / sizeof(*(arr)))

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

float magnitude(float real, float complex);
float dBSPL(float sample, float reference);
float normalize(float sample, float min, float max);
float normalize_range(float sample, struct range range);
float lerp(float start, float end, float percentage);
float lerp_range(struct range range, float percentage);
float map(float sample, float source_start, float source_end,
	  float destination_start, float destination_end);
float map_range(float sample, struct range souce, struct range destination);
float clamp(float sample, float min, float max);
float clamp_range(float sample, struct range range);

size_t index_of_frequency(float frequency);
float frequency_of_index(size_t index);
float linear_to_log(float frequency);
float get_frequency_at_point(Vector2 point);
float get_dBSPL_at_point(Vector2 point);
float approximate_normalized_left_dBSPL_at_frequency(float frequency);
float approximate_normalized_right_dBSPL_at_frequency(float frequency);
float approximate_normalized_dBSPL_at_frequency(float frequency,
						float *frequency_bins);

#endif // FRYZ_MATH_H
