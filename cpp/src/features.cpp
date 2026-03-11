#include "evensteven/features.h"
#include <cmath>
#include <algorithm>
#include <vector>
#include "kiss_fft.h"
#include "kiss_fftr.h"

namespace evensteven {

float compute_rms(const float* samples, int count) {
    if (count <= 0) return 0.0f;
    double sum = 0.0;
    for (int i = 0; i < count; i++) {
        sum += static_cast<double>(samples[i]) * static_cast<double>(samples[i]);
    }
    return static_cast<float>(std::sqrt(sum / count));
}

float compute_rms_db(const float* samples, int count) {
    float rms = compute_rms(samples, count);
    if (rms <= 0.0f) return -INFINITY;
    return 20.0f * std::log10(rms);
}

float compute_peak(const float* samples, int count) {
    float peak = 0.0f;
    for (int i = 0; i < count; i++) {
        peak = std::max(peak, std::abs(samples[i]));
    }
    return peak;
}

float compute_peak_db(const float* samples, int count) {
    float peak = compute_peak(samples, count);
    if (peak <= 0.0f) return -INFINITY;
    return 20.0f * std::log10(peak);
}

SpectralFrame compute_spectrum(const float* samples, int count, int fft_size) {
    SpectralFrame frame;
    frame.fft_size = fft_size;

    // Apply Hann window and zero-pad if needed
    std::vector<float> windowed(fft_size, 0.0f);
    int n = std::min(count, fft_size);
    for (int i = 0; i < n; i++) {
        float window = 0.5f * (1.0f - std::cos(2.0f * static_cast<float>(M_PI) * i / (n - 1)));
        windowed[i] = samples[i] * window;
    }

    // KissFFT real-to-complex
    kiss_fftr_cfg cfg = kiss_fftr_alloc(fft_size, 0, nullptr, nullptr);
    int num_bins = fft_size / 2 + 1;
    std::vector<kiss_fft_cpx> out(num_bins);
    kiss_fftr(cfg, windowed.data(), out.data());

    frame.magnitudes.resize(num_bins);
    for (int i = 0; i < num_bins; i++) {
        frame.magnitudes[i] = std::sqrt(out[i].r * out[i].r + out[i].i * out[i].i);
    }

    kiss_fft_free(cfg);
    return frame;
}

float band_energy_db(const SpectralFrame& frame, float low_hz, float high_hz, float sample_rate) {
    float freq_res = sample_rate / static_cast<float>(frame.fft_size);
    int low_bin = std::max(0, static_cast<int>(std::ceil(low_hz / freq_res)));
    int high_bin = std::min(static_cast<int>(frame.magnitudes.size()) - 1,
                            static_cast<int>(std::floor(high_hz / freq_res)));

    double energy = 0.0;
    for (int i = low_bin; i <= high_bin; i++) {
        energy += static_cast<double>(frame.magnitudes[i]) * static_cast<double>(frame.magnitudes[i]);
    }

    if (energy <= 0.0) return -INFINITY;
    return static_cast<float>(10.0 * std::log10(energy));
}

FrameIterator::FrameIterator(const AudioBuffer& buf, int frame_size, int hop_size)
    : data(buf.samples.data())
    , total_samples(static_cast<int>(buf.samples.size()))
    , frame_size(frame_size)
    , hop_size(hop_size) {}

int FrameIterator::num_frames() const {
    if (total_samples < frame_size) return 0;
    return 1 + (total_samples - frame_size) / hop_size;
}

const float* FrameIterator::frame(int index) const {
    return data + index * hop_size;
}

int FrameIterator::frame_samples(int index) const {
    int offset = index * hop_size;
    return std::min(frame_size, total_samples - offset);
}

} // namespace evensteven
