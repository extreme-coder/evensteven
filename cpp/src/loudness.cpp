#include "evensteven/loudness.h"
#include "evensteven/preprocessing.h"
#include "evensteven/features.h"
#include "evensteven/logging.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <vector>

namespace evensteven {

// Biquad filter coefficients
struct BiquadCoeffs {
    double b0, b1, b2, a1, a2;
};

static void apply_biquad(std::vector<float>& samples, const BiquadCoeffs& c) {
    double x1 = 0.0, x2 = 0.0, y1 = 0.0, y2 = 0.0;
    for (size_t i = 0; i < samples.size(); i++) {
        double x0 = static_cast<double>(samples[i]);
        double y0 = c.b0 * x0 + c.b1 * x1 + c.b2 * x2 - c.a1 * y1 - c.a2 * y2;
        x2 = x1; x1 = x0;
        y2 = y1; y1 = y0;
        samples[i] = static_cast<float>(y0);
    }
}

// K-weighting filter coefficients per ITU-R BS.1770-4
static BiquadCoeffs highshelf_48k() {
    return {1.53512485958697, -2.69169618940638, 1.19839281085285,
            -1.69065929318241, 0.73248077421585};
}

static BiquadCoeffs highpass_48k() {
    return {1.0, -2.0, 1.0,
            -1.99004745483398, 0.99007225036621};
}

static BiquadCoeffs highshelf_44100() {
    return {1.53512485958697, -2.69169618940638, 1.19839281085285,
            -1.66365511425627, 0.71238858827593};
}

static BiquadCoeffs highpass_44100() {
    return {1.0, -2.0, 1.0,
            -1.98916967823113, 0.98919185930390};
}

void apply_k_weighting(std::vector<float>& samples, int sample_rate) {
    BiquadCoeffs hs, hp;
    if (sample_rate == 48000) {
        hs = highshelf_48k();
        hp = highpass_48k();
    } else if (sample_rate == 44100) {
        hs = highshelf_44100();
        hp = highpass_44100();
    } else {
        // For other rates we use 48k coefficients (caller should resample first)
        hs = highshelf_48k();
        hp = highpass_48k();
    }
    apply_biquad(samples, hs);
    apply_biquad(samples, hp);
}

LoudnessResult compute_loudness(const AudioBuffer& audio, const Config& config) {
    LoudnessResult result;

    // Convert to mono if needed
    AudioBuffer mono = to_mono(audio);

    // Resample to 48k if not 44100 or 48000
    if (mono.sample_rate != 48000 && mono.sample_rate != 44100) {
        mono = resample(mono, 48000);
    }

    int sr = mono.sample_rate;
    int total_frames = static_cast<int>(mono.samples.size());

    // Compute RMS and peak on original signal
    result.rms_db = compute_rms_db(mono.samples.data(), total_frames);
    result.peak_db = compute_peak_db(mono.samples.data(), total_frames);

    // Band energy via FFT
    {
        int fft_size = config.fft_size;
        int hop = config.hop_size;
        FrameIterator iter(mono, fft_size, hop);
        std::map<std::string, double> band_sums;
        for (auto& [name, band] : config.bands) {
            band_sums[name] = 0.0;
        }
        int n = iter.num_frames();
        for (int i = 0; i < n; i++) {
            auto spectrum = compute_spectrum(iter.frame(i), iter.frame_samples(i), fft_size);
            for (auto& [name, band] : config.bands) {
                float e = band_energy_db(spectrum, band.min_hz, band.max_hz, static_cast<float>(sr));
                if (std::isfinite(e)) {
                    band_sums[name] += std::pow(10.0, e / 10.0);
                }
            }
        }
        for (auto& [name, sum] : band_sums) {
            if (n > 0 && sum > 0.0) {
                result.band_energy[name] = static_cast<float>(10.0 * std::log10(sum / n));
            } else {
                result.band_energy[name] = -INFINITY;
            }
        }
    }

    // Apply K-weighting for LUFS
    std::vector<float> k_weighted = mono.samples;
    apply_k_weighting(k_weighted, sr);

    // Block-based measurement
    int block_samples = sr * config.loudness.block_size_ms / 1000;
    int hop_samples = block_samples * (100 - config.loudness.overlap_percent) / 100;

    if (hop_samples <= 0) hop_samples = block_samples / 4;

    // Compute mean square per block
    std::vector<double> block_ms;
    std::vector<float> block_times;

    for (int offset = 0; offset + block_samples <= static_cast<int>(k_weighted.size()); offset += hop_samples) {
        double sum = 0.0;
        for (int j = 0; j < block_samples; j++) {
            double s = k_weighted[offset + j];
            sum += s * s;
        }
        double z = sum / block_samples;
        block_ms.push_back(z);
        block_times.push_back(static_cast<float>(offset + block_samples / 2) / sr);
    }

    // Momentary LUFS (400ms blocks, same data)
    result.momentary_lufs.reserve(block_ms.size());
    result.timestamps.reserve(block_ms.size());
    for (size_t i = 0; i < block_ms.size(); i++) {
        float lufs = (block_ms[i] > 0) ?
            static_cast<float>(-0.691 + 10.0 * std::log10(block_ms[i])) : -INFINITY;
        result.momentary_lufs.push_back(lufs);
        result.timestamps.push_back(block_times[i]);
    }

    // Short-term LUFS (3s window)
    int st_blocks = sr * config.loudness.shortterm_window_ms / 1000 / hop_samples;
    if (st_blocks < 1) st_blocks = 1;
    result.shortterm_lufs.reserve(block_ms.size());
    for (size_t i = 0; i < block_ms.size(); i++) {
        int start = std::max(0, static_cast<int>(i) - st_blocks + 1);
        double sum = 0.0;
        int count = 0;
        for (int j = start; j <= static_cast<int>(i); j++) {
            sum += block_ms[j];
            count++;
        }
        double avg = sum / count;
        float lufs = (avg > 0) ?
            static_cast<float>(-0.691 + 10.0 * std::log10(avg)) : -INFINITY;
        result.shortterm_lufs.push_back(lufs);
    }

    // Integrated LUFS with gating
    // Step 1: Absolute gate at -70 LUFS
    std::vector<double> ungated;
    for (double z : block_ms) {
        float lufs = (z > 0) ? static_cast<float>(-0.691 + 10.0 * std::log10(z)) : -INFINITY;
        if (lufs >= config.loudness.absolute_gate_lufs) {
            ungated.push_back(z);
        }
    }

    if (ungated.empty()) {
        result.integrated_lufs = -INFINITY;
        result.dynamic_range_db = 0.0f;
        return result;
    }

    // Step 2: Compute average of ungated -> Gamma_a
    double gamma_a_ms = 0.0;
    for (double z : ungated) gamma_a_ms += z;
    gamma_a_ms /= ungated.size();
    float gamma_a = static_cast<float>(-0.691 + 10.0 * std::log10(gamma_a_ms));

    // Step 3: Relative gate = Gamma_a - 10 LU
    float relative_gate = gamma_a + config.loudness.relative_gate_offset_lu;

    // Step 4: Filter blocks below relative gate
    std::vector<double> gated;
    for (double z : block_ms) {
        float lufs = (z > 0) ? static_cast<float>(-0.691 + 10.0 * std::log10(z)) : -INFINITY;
        if (lufs >= relative_gate) {
            gated.push_back(z);
        }
    }

    if (gated.empty()) {
        result.integrated_lufs = -INFINITY;
    } else {
        double mean_z = 0.0;
        for (double z : gated) mean_z += z;
        mean_z /= gated.size();
        result.integrated_lufs = static_cast<float>(-0.691 + 10.0 * std::log10(mean_z));
    }

    // Dynamic range: 95th - 10th percentile of short-term LUFS
    {
        std::vector<float> st_sorted;
        for (float v : result.shortterm_lufs) {
            if (std::isfinite(v)) st_sorted.push_back(v);
        }
        if (st_sorted.size() >= 2) {
            std::sort(st_sorted.begin(), st_sorted.end());
            int p10 = static_cast<int>(st_sorted.size() * 0.1f);
            int p95 = static_cast<int>(st_sorted.size() * 0.95f);
            p95 = std::min(p95, static_cast<int>(st_sorted.size()) - 1);
            result.dynamic_range_db = st_sorted[p95] - st_sorted[p10];
        }
    }

    return result;
}

} // namespace evensteven
