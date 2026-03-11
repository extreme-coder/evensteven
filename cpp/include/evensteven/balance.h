#pragma once

#include <vector>
#include "audio_io.h"
#include "config.h"

namespace evensteven {

struct BalanceResult {
    float vocal_presence_score = 0.0f;
    float masking_risk_score = 0.0f;
    std::vector<float> vocal_energy_db;
    std::vector<float> accompaniment_energy_db;
    std::vector<float> balance_db;
    std::vector<bool> masking_risk;
    std::vector<float> timestamps;
};

BalanceResult compute_balance(const AudioBuffer& audio, const Config& config);

} // namespace evensteven
