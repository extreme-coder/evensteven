#pragma once

#include <vector>
#include "audio_io.h"

namespace evensteven {

struct SpectralFrame {
    std::vector<float> magnitudes;
    float freq_resolution;
    int fft_size;
};

float compute_rms(const float* samples, int count);
float compute_rms_db(const float* samples, int count);
float compute_peak(const float* samples, int count);
float compute_peak_db(const float* samples, int count);
SpectralFrame compute_spectrum(const float* samples, int count, int fft_size);
float band_energy_db(const SpectralFrame& frame, float low_hz, float high_hz, float sample_rate);

struct FrameIterator {
    const float* data;
    int total_samples;
    int frame_size;
    int hop_size;

    FrameIterator(const AudioBuffer& buf, int frame_size, int hop_size);

    int num_frames() const;
    const float* frame(int index) const;
    int frame_samples(int index) const;
};

} // namespace evensteven
