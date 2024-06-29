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

void toggle_mute(void)
{
	if (fryz->audio.muted)
	{
		SetMusicVolume(fryz->audio.music, fryz->audio.volume);
		TraceLog(LOG_INFO, "Music unmuted");
	}
	else
	{
		SetMusicVolume(fryz->audio.music, 0);
		TraceLog(LOG_INFO, "Music muted");
	}
	fryz->audio.muted = !fryz->audio.muted;
}

void toggle_pause(void)
{
	if (fryz->audio.paused)
	{
		ResumeMusicStream(fryz->audio.music);
		TraceLog(LOG_INFO, "Music resumed play");
	}
	else
	{
		PauseMusicStream(fryz->audio.music);
		TraceLog(LOG_INFO, "Music paused");
	}
	fryz->audio.paused = !fryz->audio.paused;
}

// Doesn't update music when paused
void skip(float dt)
{
	float current_time = GetMusicTimePlayed(fryz->audio.music);
	float music_length = GetMusicTimeLength(fryz->audio.music);
	float time = clamp(current_time + dt, 0.0, music_length);
	// static bool logged = false;
	// if (!logged)
	// {
	// 	TraceLog(LOG_DEBUG, "%f, %f, %f, %f",
	// 		 dt, current_time, music_length, time);
	// }
	SeekMusicStream(fryz->audio.music, time); // Doesn't work at extremes
}

void set_fft_size(size_t size)
{
	size = MAX(size, MIN_FFT_SIZE);
	size = MIN(size, MAX_FFT_SIZE);
	fft_size = size;
}

int set_fft_size_and_init_data_sets(size_t size)
{
	set_fft_size(size);
	max_sample_size = fft_size / 2;
	return init_fft_data_sets();
}

int  init_fft_data_sets(void)
{
	int error;
	error = init_fft_data(&left);
	if (error)
	{
		TraceLog(LOG_ERROR, "Failed to initialize left fft data"
			 " (fft_size=%ul, error=%d)", fft_size, error);
		return error;
	}
	error = init_fft_data(&right);
	if (error)
	{
		TraceLog(LOG_ERROR, "Failed to initialize right fft data"
			 " (fft_size=%ul, error=%d)", fft_size, error);
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
	data->waveform = fftwf_alloc_real(fft_size);
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
	data->plan = fftwf_plan_dft_1d(fft_size, data->waveform_windowed,
				       data->out, FFTW_FORWARD, FFTW_ESTIMATE);
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

void raylib_waveform_capture_callback(void *bufferData, unsigned int frames)
{
	if (frames == 0 || bufferData == NULL)
		return;

	float *waveform = bufferData;
	frames = MIN(frames, (unsigned int)fft_size);
	size_t insert_start = fft_size - frames;

	// Shift the saved waveform data to the left so that the new data
	// can be pushed on
	memmove(left.waveform,  left.waveform + frames,
		insert_start * sizeof(float));
	memmove(right.waveform, right.waveform + frames,
		insert_start * sizeof(float));

	// Push the new data
	for (unsigned int i = 0; i < frames; ++i)
	{
		float left_sample  = waveform[i * 2 + 0];
		float right_sample = waveform[i * 2 + 1];
		left.waveform[insert_start  + i] = left_sample;
		right.waveform[insert_start + i] = right_sample;
	}

	/* process_audio(); */
}

void process_audio(void)
{
	apply_window_to_waveform_data();
	preform_fft();
	take_magnitude_of_frequency_bins();
	convert_raw_data_to_dBSPL();
	// smooth_frequency_bins();
	normalize_frequency_bins();
}

void apply_window_to_waveform_data(void)
{
	for (size_t i = 0; i < fft_size; ++i)
	{
		float left_raw = left.waveform[i];
		float right_raw = right.waveform[i];
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
		right.frequency_bins[i] = magnitude(right_real,
						    right_imaginary);
	}
}

// Actually dBFS
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
		TraceLog(LOG_WARNING, "Failed to smooth: "
			 "Couldn't allocate memory for smoothing");
		return;
	}
	float *right_buffer = calloc(fft_size, sizeof(float));
	if (right_buffer == NULL)
	{
		TraceLog(LOG_WARNING, "Failed to smooth: "
			 "Couldn't allocate memory for smoothing");
		free(left_buffer);
		return;
	}

	// Moving average
	int di = 20;
	for (size_t i = index_of_frequency(20);
	     i < index_of_frequency(20000); ++i)
	{
		long double left_average = 0;
		long double right_average = 0;
		right.frequency_bins[i] = left.frequency_bins[i];

		size_t start = MAX(0, (int)i - di);
		size_t end = MIN((int)fft_size, (int)i + di);
		for (size_t j = start; j <= end; ++j)
		{
			left_average += left.frequency_bins[j];
			right_average += right.frequency_bins[j];
		}

		left_buffer[i] = (float) left_average / (1 + end - start);
		right_buffer[i] = (float) right_average / (1 + end - start);
	}


	memcpy(left.frequency_bins, left_buffer, sizeof(float) * fft_size);
	memcpy(right.frequency_bins, right_buffer, sizeof(float) * fft_size);

	free(left_buffer);
	free(right_buffer);
}

void normalize_frequency_bins(void)
{
	for (size_t i = 0; i < fft_size; ++i)
	{
		float left_smoothed  = left.frequency_bins[i];
		float right_smoothed = right.frequency_bins[i];
		left.frequency_bins[i] = normalize_range(left_smoothed,
							 fryz->audio.range);
		right.frequency_bins[i] = normalize_range(right_smoothed,
							  fryz->audio.range);
	}
}
