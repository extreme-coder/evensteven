#pragma once

#include <vector>
#include <map>
#include <string>
#include "audio_io.h"
#include "config.h"

namespace evensteven {

struct LoudnessResult {
    float integrated_lufs = -INFINITY;
    std::vector<float> shortterm_lufs;
    std::vector<float> momentary_lufs;
    float rms_db = -INFINITY;
    float peak_db = -INFINITY;
    float dynamic_range_db = 0.0f;
    std::map<std::string, float> band_energy;
    std::vector<float> timestamps;
};

void apply_k_weighting(std::vector<float>& samples, int sample_rate);
LoudnessResult compute_loudness(const AudioBuffer& audio, const Config& config);

} // namespace evensteven
