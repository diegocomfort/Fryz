#include "../include/audio.h"
#include "../include/fryz.h"
#include "../include/math.h"

#include <raylib.h>
#include <fftw3.h>

#include <math.h>
#include <string.h>

#define fft_size        fryz->audio.fft_size
#define left            fryz->audio.left
#define right           fryz->audio.right
#define max_sample_size fryz->audio.max_sample_size

extern struct fryz *fryz;

void set_fft_size(size_t size)
{
    size = MAX(size, MIN_FFT_SIZE);
    size = MIN(size, MAX_FFT_SIZE);
    fft_size = size;
    fft_size = 1 + size / 2;
}

int set_fft_size_and_init_data_sets(size_t size)
{
    set_fft_size(size);
    return init_fft_data_sets();
}

int  init_fft_data_sets(void)
{
    int error;
    error = init_fft_data(&left);
    if (error)
    {
	TraceLog(LOG_ERROR,
		 "Failed to initialize left fft data (fft_size=%ul, error=%d)",
		 fft_size, error);
	return error;
    }
    error = init_fft_data(&right);
    if (error)
    {
	TraceLog(LOG_ERROR,
		 "Failed to initialize right fft data (fft_size=%ul, error=%d)",
		 fft_size, error);
	return error;
    }
    return 0;
}

void close_fft_data_sets(void)
{
    close_fft_data(&left);
    close_fft_data(&right);
}

int init_fft_data(void *fft_data)
{
    struct fft *data = fft_data;

    // Waveform
    if (data->waveform != NULL)
	return 1;
    data->waveform = fftwf_alloc_complex(fft_size);
    if (data->waveform == NULL)
	return 2;

    // Waveform windowed
    if (data->waveform_windowed != NULL)
	return 1;
    data->waveform_windowed = fftwf_alloc_complex(fft_size);
    if (data->waveform_windowed == NULL)
	return 2;

    // Complex output
    if (data->out != NULL)
	return 1;
    data->out = fftwf_alloc_complex(fft_size);
    if (data->out == NULL)
	return 2;

    // Frequency bins
    if (data->frequency_bins != NULL)
	return 1;
    data->frequency_bins = fftwf_alloc_real(fft_size);
    if (data->frequency_bins == NULL)
	return 2;

    // FFTW Plan
    if (data->plan != NULL)
	return 3;
    data->plan = fftwf_plan_dft_c2r_1d(fft_size, data->waveform_windowed, data->frequency_bins, FFTW_ESTIMATE);
    if (data->plan == NULL)
	return 4;

    return 0;
}

void close_fft_data(void *fft_data)
{
    struct fft *data = fft_data;
    
    if (data->waveform != NULL)
    {
	fftwf_free(data->waveform);
	data->waveform = NULL;
    }

    if (data->waveform_windowed != NULL)
    {
	fftwf_free(data->waveform_windowed);
	data->waveform_windowed = NULL;
    }

    if (data->out != NULL)
    {
	fftwf_free(data->out);
	data->out = NULL;
    }

    if (data->frequency_bins != NULL)
    {
	fftwf_free(data->frequency_bins);
	data->frequency_bins = NULL;
    }

    if (data->plan != NULL)
    {
	fftwf_destroy_plan(data->plan);
	data->plan = NULL;
    }
}

void set_max_sample_size()
{
    size_t sample_size = fryz->audio.music.stream.sampleSize;
    switch (sample_size)
    {
    case 8:
        max_sample_size = 255;
        return;
    case 16:
        max_sample_size = 32767; // (1 << (16 - 1)) - 1
        return;
    case 32:
        max_sample_size = 2147483647; // (1 << (32 - 1)) - 1
        return;
    default:
        TraceLog(LOG_WARNING, "The sample rate of the music is neither 8, 16, nor 32: (actually %d)", sample_size);
        max_sample_size = 1000; // Shouldn't happen
        return;
    }
}

void raylib_waveform_capture_callback(void *bufferData, unsigned int frames)
{
    float *waveform = bufferData;
    frames = MIN(frames, (unsigned int)fft_size);
    size_t insert_start = fft_size - frames;

    // Shift the saved waveform data to the left so that the new data
    // can be pushed on
    memmove(left.waveform,  left.waveform + frames, insert_start * sizeof(fftwf_complex));
    memmove(right.waveform, right.waveform + frames, insert_start * sizeof(fftwf_complex));

    // Push the new data
    for (unsigned int i = 0; i < frames; ++i)
    {
        float left_sample  = waveform[i * 2 + 0];
        float right_sample = waveform[i * 2 + 1];
        left.waveform[insert_start  + i][0] = left_sample;
        right.waveform[insert_start + i][0] = right_sample;
    }

    process_audio();
}

void process_audio(void)
{
    apply_window_to_waveform_data();
    preform_fft();
    take_magnitude_of_frequency_bins();
    convert_raw_data_to_dBSPL();
    smooth_frequency_bins();
    normalize_frequency_bins();
}

void apply_window_to_waveform_data(void)
{
    for (size_t i = 0; i < fft_size; ++i)
    {
        float left_raw = left.waveform[i][0];
        float right_raw = right.waveform[i][0];
        float tmp = (float) i / (fft_size - 1);
        float hann = 0.5 * (1 - cosf(2 * PI * tmp));
        left.waveform_windowed[i][0] = left_raw * hann;
        right.waveform_windowed[i][0] = right_raw * hann;
    }
}

void preform_fft(void)
{
    fftwf_execute(left.plan);
    fftwf_execute(right.plan);
}

void take_magnitude_of_frequency_bins(void)
{
    for (size_t i = 0; i < fft_size; ++i)
    {
        float left_real = left.out[i][0];
	float left_imaginary = left.out[i][1];
	float right_real = right.out[i][0];
	float right_imaginary = right.out[i][1];
        left.frequency_bins[i] = magnitude(left_real, left_imaginary);
        right.frequency_bins[i] = magnitude(right_real, right_imaginary);
    }
}

void convert_raw_data_to_dBSPL(void)
{
    for (size_t i = 0; i < fft_size; ++i)
    {
        float left_abs = left.frequency_bins[i];
        float right_abs = right.frequency_bins[i];
        left.frequency_bins[i] = dBSPL(left_abs, max_sample_size);
        right.frequency_bins[i] = dBSPL(right_abs, max_sample_size);
    }
}

void smooth_frequency_bins(void)
{
    // Allocate buffers
    float *left_buffer = calloc(fft_size, sizeof(float));
    if (left_buffer == NULL)
    {
        TraceLog(LOG_WARNING, "Couldn't allocate memory for smoothing");
        return;
    }
    float *right_buffer = calloc(fft_size, sizeof(float));
    if (right_buffer == NULL)
    {
        TraceLog(LOG_WARNING, "Couldn't allocate memory for smoothing");
        free(left_buffer);
        return;
    }

    // Smooth
    // https://en.wikipedia.org/wiki/Exponential_smoothing
    left_buffer[0] = left.frequency_bins[0];
    right_buffer[0] = right.frequency_bins[0];
    for (size_t i = 1; i < fft_size; ++i)
    {
        float left_previous = left.frequency_bins[i - 1];
        float right_previous = right.frequency_bins[i - 1];
        float left_current = left.frequency_bins[i];
        float right_current = right.frequency_bins[i];
        left.frequency_bins[i] = (1 - left.smoothing_factor) * left_current + left.smoothing_factor * left_previous;
        right.frequency_bins[i] = (1 - right.smoothing_factor) * right_current + right.smoothing_factor * right_previous;
    }

    free(left_buffer);
    free(right_buffer);
}

void normalize_frequency_bins(void)
{
    for (size_t i = 0; i < fft_size; ++i)
    {
        float left_smoothed  = left.frequency_bins[i];
        float right_smoothed = right.frequency_bins[i];
        left.frequency_bins[i] = normalize_range(left_smoothed, fryz->audio.range);
        right.frequency_bins[i] = normalize_range(right_smoothed, fryz->audio.range);
    }
}
