#include "evensteven/config.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>

namespace evensteven {

using json = nlohmann::json;

Config Config::load_from_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open config file: " + path);
    }

    json j;
    file >> j;

    Config cfg = default_config();

    if (j.contains("sample_rate")) cfg.sample_rate = j["sample_rate"];
    if (j.contains("frame_size")) cfg.frame_size = j["frame_size"];
    if (j.contains("hop_size")) cfg.hop_size = j["hop_size"];
    if (j.contains("fft_size")) cfg.fft_size = j["fft_size"];

    if (j.contains("loudness")) {
        auto& l = j["loudness"];
        if (l.contains("block_size_ms")) cfg.loudness.block_size_ms = l["block_size_ms"];
        if (l.contains("overlap_percent")) cfg.loudness.overlap_percent = l["overlap_percent"];
        if (l.contains("absolute_gate_lufs")) cfg.loudness.absolute_gate_lufs = l["absolute_gate_lufs"];
        if (l.contains("relative_gate_offset_lu")) cfg.loudness.relative_gate_offset_lu = l["relative_gate_offset_lu"];
        if (l.contains("shortterm_window_ms")) cfg.loudness.shortterm_window_ms = l["shortterm_window_ms"];
        if (l.contains("momentary_window_ms")) cfg.loudness.momentary_window_ms = l["momentary_window_ms"];
    }

    if (j.contains("balance")) {
        auto& b = j["balance"];
        if (b.contains("vocal_low_hz")) cfg.balance.vocal_low_hz = b["vocal_low_hz"];
        if (b.contains("vocal_high_hz")) cfg.balance.vocal_high_hz = b["vocal_high_hz"];
        if (b.contains("masking_threshold_db")) cfg.balance.masking_threshold_db = b["masking_threshold_db"];
    }

    if (j.contains("bands")) {
        cfg.bands.clear();
        for (auto& [key, val] : j["bands"].items()) {
            cfg.bands[key] = {val["min_hz"], val["max_hz"]};
        }
    }

    if (j.contains("segmentation")) {
        auto& s = j["segmentation"];
        if (s.contains("smoothing_window_s")) cfg.segmentation.smoothing_window_s = s["smoothing_window_s"];
        if (s.contains("energy_change_threshold_db")) cfg.segmentation.energy_change_threshold_db = s["energy_change_threshold_db"];
        if (s.contains("silence_threshold_db")) cfg.segmentation.silence_threshold_db = s["silence_threshold_db"];
        if (s.contains("min_silence_duration_s")) cfg.segmentation.min_silence_duration_s = s["min_silence_duration_s"];
        if (s.contains("min_segment_duration_s")) cfg.segmentation.min_segment_duration_s = s["min_segment_duration_s"];
    }

    if (j.contains("scoring")) {
        auto& sc = j["scoring"];
        if (sc.contains("good_threshold")) cfg.scoring.good_threshold = sc["good_threshold"];
        if (sc.contains("fair_threshold")) cfg.scoring.fair_threshold = sc["fair_threshold"];
        if (sc.contains("target_lufs")) cfg.scoring.target_lufs = sc["target_lufs"];
        if (sc.contains("lufs_tolerance")) cfg.scoring.lufs_tolerance = sc["lufs_tolerance"];
    }

    if (j.contains("export")) {
        auto& e = j["export"];
        if (e.contains("format")) cfg.export_cfg.format = e["format"].get<std::string>();
        if (e.contains("include_timelines")) cfg.export_cfg.include_timelines = e["include_timelines"];
        if (e.contains("timeline_resolution_ms")) cfg.export_cfg.timeline_resolution_ms = e["timeline_resolution_ms"];
    }

    return cfg;
}

Config Config::default_config() {
    Config cfg;
    cfg.bands["low"] = {20.0f, 250.0f};
    cfg.bands["mid"] = {250.0f, 4000.0f};
    cfg.bands["high"] = {4000.0f, 20000.0f};
    return cfg;
}

} // namespace evensteven
