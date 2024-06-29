#include "../include/math.h"
#include "../include/fryz.h"

#include <raylib.h>

#include <math.h>

#define fft_size    fryz->audio.fft_size
#define sample_rate fryz->audio.music.stream.sampleRate
#define viewport    fryz->graph.viewport
#define domain		fryz->audio.domain
#define dBSPL_range	fryz->audio.range

extern struct fryz *fryz;

float magnitude(float real, float complex)
{
	return sqrtf(real * real + complex * complex);
}

float dBSPL(float sample, float reference)
{
	return 20 * log10f(sample / reference);
}

float normalize(float sample, float min, float max)
{
	if (sample <= min)
		return 0;
	if (sample >= max)
		return 1;
	return (sample - min) / (max - min);
}

float normalize_range(float sample, struct range range)
{
	return normalize(sample, range.min, range.max);
}

// NOTE: lerp and normalize are inverses
float lerp(float start, float end, float percentage)
{
	return start + percentage * (end - start);
}

float lerp_range(struct range range, float percentage)
{
	return lerp(range.min, range.max, percentage);
}

float map(float sample, float source_start, float source_end,
	  float destination_start, float destination_end)
{
	float percentage = normalize(sample, source_start, source_end);
	return lerp(destination_start, destination_end, percentage);
}

float map_range(float sample, struct range source, struct range destination)
{
	return map(sample, source.min, source.max,
		   destination.min, destination.max);
}

float clamp(float sample, float min, float max)
{
	if (sample < min)
	{
		return min;
	}
	if (sample > max)
	{
		return max;
	}
	return sample;
}

float clamp_range(float sample, struct range range)
{
	return clamp(sample, range.min, range.max);
}

size_t index_of_frequency(float frequency)
{
	frequency = fabsf(frequency);
	return frequency * fft_size / sample_rate;
}

float frequency_of_index(size_t index)
{
	return index * sample_rate / fft_size;
}

float linear_to_log(float frequency)
{
	// linear_to_log(20) == 0
	// linear_to_log(20000) == 1
	return log10f(frequency / 20) / 3;
}

float log_to_linear(float frequency)
{
	// log_to_linear(0) == 20
	// log_to_linear(1) == 20000
	return 20 * powf(10, 3 * frequency);
}

// TODO: Make more understandable and less shitty
// (more variables with acurate names)
float get_frequency_at_point(Vector2 point)
{
	// Inverse of algorithm in project_sample()
	float frequency = (point.x - viewport.x) / viewport.width;
	float min = linear_to_log(domain.min);
	float max = linear_to_log(domain.max);
	frequency = lerp(min, max, frequency);
	frequency = log_to_linear(frequency);
	return frequency;
}

float get_dBSPL_at_point(Vector2 point)
{
	float normalized_sample = (viewport.y + viewport.height - point.y) /
		(viewport.height);
	float dBSPL = lerp_range(dBSPL_range, normalized_sample);
	return dBSPL;
}

float approximate_normalized_left_dBSPL_at_frequency(float frequency)
{
	return approximate_normalized_dBSPL_at_frequency(
		frequency, fryz->audio.left.frequency_bins);
}

float approximate_normalized_right_dBSPL_at_frequency(float frequency)
{
	return approximate_normalized_dBSPL_at_frequency(
		frequency, fryz->audio.right.frequency_bins);
}

float approximate_normalized_dBSPL_at_frequency(float frequency,
						float *frequency_bins)
{
	size_t lower_index = index_of_frequency(frequency);
	size_t upper_index = 1 + lower_index;
	float  lower_frequency = frequency_of_index(lower_index);
	float  upper_frequency = frequency_of_index(upper_index);
	float  lower_dBSPL = frequency_bins[lower_index];
	float  upper_dBSPL = frequency_bins[upper_index];

	float percentage = normalize(frequency, lower_frequency,
				     upper_frequency);
	return lerp(lower_dBSPL, upper_dBSPL, percentage);
}
