#include "evensteven/segmentation.h"
#include "evensteven/preprocessing.h"
#include "evensteven/features.h"
#include "evensteven/logging.h"
#include <cmath>
#include <algorithm>

namespace evensteven {

std::vector<Section> auto_segment(const AudioBuffer& audio, const Config& config) {
    AudioBuffer mono = to_mono(audio);
    int sr = mono.sample_rate;
    float duration = mono.duration_s();

    // Compute RMS per frame (100ms frames)
    int frame_size = sr / 10;
    int hop = frame_size;
    int num_frames = static_cast<int>(mono.samples.size()) / hop;

    if (num_frames < 2) {
        return {{.label = "section_1", .start_s = 0.0f, .end_s = duration}};
    }

    std::vector<float> rms(num_frames);
    for (int i = 0; i < num_frames; i++) {
        int offset = i * hop;
        int count = std::min(frame_size, static_cast<int>(mono.samples.size()) - offset);
        rms[i] = compute_rms_db(mono.samples.data() + offset, count);
        if (!std::isfinite(rms[i])) rms[i] = -96.0f;
    }

    // Smooth with moving average
    int smooth_frames = static_cast<int>(config.segmentation.smoothing_window_s * 10);
    std::vector<float> smoothed(num_frames);
    for (int i = 0; i < num_frames; i++) {
        int start = std::max(0, i - smooth_frames / 2);
        int end = std::min(num_frames, i + smooth_frames / 2 + 1);
        float sum = 0.0f;
        for (int j = start; j < end; j++) sum += rms[j];
        smoothed[i] = sum / (end - start);
    }

    // Detect boundaries: frames where energy change exceeds threshold
    std::vector<float> boundaries;
    boundaries.push_back(0.0f);

    float threshold = config.segmentation.energy_change_threshold_db;
    float silence_thresh = config.segmentation.silence_threshold_db;
    float min_silence = config.segmentation.min_silence_duration_s;
    float min_segment = config.segmentation.min_segment_duration_s;

    // Detect silence regions
    bool in_silence = false;
    float silence_start = 0.0f;

    for (int i = 1; i < num_frames; i++) {
        float time = static_cast<float>(i) * 0.1f;
        float diff = std::abs(smoothed[i] - smoothed[i - 1]);

        if (smoothed[i] < silence_thresh) {
            if (!in_silence) {
                in_silence = true;
                silence_start = time;
            }
        } else {
            if (in_silence && (time - silence_start) >= min_silence) {
                boundaries.push_back(silence_start);
                boundaries.push_back(time);
            }
            in_silence = false;
        }

        if (diff > threshold && !in_silence) {
            boundaries.push_back(time);
        }
    }

    if (in_silence && (duration - silence_start) >= min_silence) {
        boundaries.push_back(silence_start);
    }

    boundaries.push_back(duration);

    // Sort and deduplicate
    std::sort(boundaries.begin(), boundaries.end());
    boundaries.erase(std::unique(boundaries.begin(), boundaries.end()), boundaries.end());

    // Remove boundaries too close together
    std::vector<float> filtered;
    filtered.push_back(boundaries[0]);
    for (size_t i = 1; i < boundaries.size(); i++) {
        if (boundaries[i] - filtered.back() >= min_segment) {
            filtered.push_back(boundaries[i]);
        }
    }
    if (filtered.back() < duration) {
        filtered.push_back(duration);
    }

    // Build sections
    std::vector<Section> sections;
    for (size_t i = 0; i + 1 < filtered.size(); i++) {
        Section sec;
        sec.label = "section_" + std::to_string(i + 1);
        sec.start_s = filtered[i];
        sec.end_s = filtered[i + 1];

        // Check if this section is mostly silence
        int start_frame = static_cast<int>(sec.start_s * 10);
        int end_frame = std::min(static_cast<int>(sec.end_s * 10), num_frames);
        int silent_count = 0;
        for (int f = start_frame; f < end_frame; f++) {
            if (smoothed[f] < silence_thresh) silent_count++;
        }
        sec.is_silence = (end_frame > start_frame) &&
                         (static_cast<float>(silent_count) / (end_frame - start_frame) > 0.8f);

        sections.push_back(sec);
    }

    if (sections.empty()) {
        sections.push_back({.label = "section_1", .start_s = 0.0f, .end_s = duration});
    }

    return sections;
}

AudioBuffer extract_section(const AudioBuffer& audio, const Section& section) {
    AudioBuffer out;
    out.sample_rate = audio.sample_rate;
    out.channels = audio.channels;

    int start_sample = static_cast<int>(section.start_s * audio.sample_rate) * audio.channels;
    int end_sample = static_cast<int>(section.end_s * audio.sample_rate) * audio.channels;
    start_sample = std::max(0, std::min(start_sample, static_cast<int>(audio.samples.size())));
    end_sample = std::max(start_sample, std::min(end_sample, static_cast<int>(audio.samples.size())));

    out.samples.assign(audio.samples.begin() + start_sample, audio.samples.begin() + end_sample);
    return out;
}

std::vector<Section> merge_sections(const std::vector<Section>& user_sections,
                                     const std::vector<Section>& auto_sections) {
    if (!user_sections.empty()) return user_sections;
    return auto_sections;
}

} // namespace evensteven
