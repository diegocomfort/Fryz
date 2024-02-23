#ifndef FRYZ_FFT_H
#define FRYZ_FFT_H

#include "fryz.h"

#include <fftw3.h>

#define MIN_FFT_SIZE 512
#define MAX_FFT_SIZE (1UL << 32) // Why?

void set_fft_size(size_t size);
int  set_fft_size_and_init_data_sets(size_t size);

int  init_fft_data_sets(void);
void close_fft_data_sets(void);
int  init_fft_data(void *fft_data);  // paremeter is struct fft *
void close_fft_data(void *fft_data); // paremeter is struct fft *

void set_max_sample_size(void);
void raylib_waveform_capture_callback(void *bufferData, unsigned int frames);

void process_audio(void);
void apply_window_to_waveform_data(void);
void preform_fft(void);
void take_magnitude_of_frequency_bins(void);
void convert_raw_data_to_dBSPL(void);
void smooth_frequency_bins(void);
void normalize_frequency_bins(void);

#endif // FRYZ_FFT_H
