#ifndef FRYZ_MATH_H
#define FRYZ_MATH_H

struct range
{
    float min;
    float max;
};

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

float magnitude(float real, float complex);
float dBSPL(float sample, float reference);
float normalize(float sample, float min, float max);
float normalize_range(float sample, struct range range);

#endif // FRYZ_MATH_H
