#include "evensteven/preprocessing.h"
#include "evensteven/features.h"
#include <cmath>
#include <algorithm>
#include <numeric>

namespace evensteven {

AudioBuffer to_mono(const AudioBuffer& buf) {
    if (buf.channels == 1) return buf;

    AudioBuffer mono;
    mono.sample_rate = buf.sample_rate;
    mono.channels = 1;
    int frames = buf.num_frames();
    mono.samples.resize(frames);

    for (int i = 0; i < frames; i++) {
        float sum = 0.0f;
        for (int ch = 0; ch < buf.channels; ch++) {
            sum += buf.samples[i * buf.channels + ch];
        }
        mono.samples[i] = sum / static_cast<float>(buf.channels);
    }

    return mono;
}

AudioBuffer resample(const AudioBuffer& buf, int target_rate) {
    if (buf.sample_rate == target_rate) return buf;

    AudioBuffer out;
    out.sample_rate = target_rate;
    out.channels = buf.channels;

    double ratio = static_cast<double>(target_rate) / static_cast<double>(buf.sample_rate);
    int in_frames = buf.num_frames();
    int out_frames = static_cast<int>(std::ceil(in_frames * ratio));
    out.samples.resize(out_frames * buf.channels);

    for (int ch = 0; ch < buf.channels; ch++) {
        for (int i = 0; i < out_frames; i++) {
            double src_pos = i / ratio;
            int idx = static_cast<int>(src_pos);
            double frac = src_pos - idx;

            int idx0 = std::min(idx, in_frames - 1);
            int idx1 = std::min(idx + 1, in_frames - 1);

            float s0 = buf.samples[idx0 * buf.channels + ch];
            float s1 = buf.samples[idx1 * buf.channels + ch];
            out.samples[i * buf.channels + ch] = static_cast<float>(s0 + frac * (s1 - s0));
        }
    }

    return out;
}

AudioBuffer normalize_peak(const AudioBuffer& buf, float target) {
    AudioBuffer out = buf;
    float peak = 0.0f;
    for (float s : out.samples) {
        peak = std::max(peak, std::abs(s));
    }
    if (peak > 0.0f) {
        float gain = target / peak;
        for (float& s : out.samples) {
            s *= gain;
        }
    }
    return out;
}

float estimate_noise_floor_db(const AudioBuffer& buf) {
    AudioBuffer mono = to_mono(buf);
    int frame_size = mono.sample_rate / 10; // 100ms frames
    int hop = frame_size;
    int num_frames = static_cast<int>(mono.samples.size()) / hop;

    if (num_frames == 0) return -96.0f;

    std::vector<float> rms_values;
    rms_values.reserve(num_frames);

    for (int i = 0; i < num_frames; i++) {
        int offset = i * hop;
        int count = std::min(frame_size, static_cast<int>(mono.samples.size()) - offset);
        float rms = compute_rms_db(mono.samples.data() + offset, count);
        if (std::isfinite(rms)) {
            rms_values.push_back(rms);
        }
    }

    if (rms_values.empty()) return -96.0f;

    std::sort(rms_values.begin(), rms_values.end());
    // 10th percentile as noise floor estimate
    int idx = static_cast<int>(rms_values.size() * 0.1f);
    return rms_values[idx];
}

} // namespace evensteven
