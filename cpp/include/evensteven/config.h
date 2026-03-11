#pragma once

#include <string>
#include <map>

namespace evensteven {

struct BandConfig {
    float min_hz;
    float max_hz;
};

struct LoudnessConfig {
    int block_size_ms = 400;
    int overlap_percent = 75;
    float absolute_gate_lufs = -70.0f;
    float relative_gate_offset_lu = -10.0f;
    int shortterm_window_ms = 3000;
    int momentary_window_ms = 400;
};

struct BalanceConfig {
    float vocal_low_hz = 300.0f;
    float vocal_high_hz = 3400.0f;
    float masking_threshold_db = -3.0f;
};

struct SegmentationConfig {
    float smoothing_window_s = 2.0f;
    float energy_change_threshold_db = 6.0f;
    float silence_threshold_db = -40.0f;
    float min_silence_duration_s = 1.0f;
    float min_segment_duration_s = 5.0f;
};

struct ScoringConfig {
    float good_threshold = 0.7f;
    float fair_threshold = 0.4f;
    float target_lufs = -14.0f;
    float lufs_tolerance = 2.0f;
};

struct ExportConfig {
    std::string format = "both";
    bool include_timelines = true;
    int timeline_resolution_ms = 100;
};

struct Config {
    int sample_rate = 48000;
    int frame_size = 4096;
    int hop_size = 1024;
    int fft_size = 4096;

    LoudnessConfig loudness;
    BalanceConfig balance;
    std::map<std::string, BandConfig> bands;
    SegmentationConfig segmentation;
    ScoringConfig scoring;
    ExportConfig export_cfg;

    static Config load_from_file(const std::string& path);
    static Config default_config();
};

} // namespace evensteven
