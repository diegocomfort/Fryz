#include "../include/math.h"

#include <math.h>

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
