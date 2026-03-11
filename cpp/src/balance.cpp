#include "evensteven/balance.h"
#include "evensteven/preprocessing.h"
#include "evensteven/features.h"
#include "evensteven/logging.h"
#include <cmath>
#include <algorithm>

namespace evensteven {

BalanceResult compute_balance(const AudioBuffer& audio, const Config& config) {
    BalanceResult result;

    AudioBuffer mono = to_mono(audio);

    int sr = mono.sample_rate;
    int fft_size = config.fft_size;
    int hop = config.hop_size;

    FrameIterator iter(mono, fft_size, hop);
    int n = iter.num_frames();

    if (n == 0) return result;

    result.vocal_energy_db.reserve(n);
    result.accompaniment_energy_db.reserve(n);
    result.balance_db.reserve(n);
    result.masking_risk.reserve(n);
    result.timestamps.reserve(n);

    float vocal_low = config.balance.vocal_low_hz;
    float vocal_high = config.balance.vocal_high_hz;
    float threshold = config.balance.masking_threshold_db;

    double vocal_sum = 0.0;
    double total_sum = 0.0;
    int masked_frames = 0;

    for (int i = 0; i < n; i++) {
        auto spectrum = compute_spectrum(iter.frame(i), iter.frame_samples(i), fft_size);

        // Vocal band energy
        float vocal_db = band_energy_db(spectrum, vocal_low, vocal_high, static_cast<float>(sr));
        // Total energy
        float total_db = band_energy_db(spectrum, 20.0f, 20000.0f, static_cast<float>(sr));
        // Accompaniment = total - vocal (in linear domain)
        double vocal_lin = std::isfinite(vocal_db) ? std::pow(10.0, vocal_db / 10.0) : 0.0;
        double total_lin = std::isfinite(total_db) ? std::pow(10.0, total_db / 10.0) : 0.0;
        double accomp_lin = std::max(0.0, total_lin - vocal_lin);

        float accomp_db = (accomp_lin > 0) ? static_cast<float>(10.0 * std::log10(accomp_lin)) : -INFINITY;
        float balance = vocal_db - accomp_db;

        result.vocal_energy_db.push_back(vocal_db);
        result.accompaniment_energy_db.push_back(accomp_db);
        result.balance_db.push_back(std::isfinite(balance) ? balance : 0.0f);

        bool masked = std::isfinite(balance) && balance < threshold;
        result.masking_risk.push_back(masked);
        if (masked) masked_frames++;

        float time = static_cast<float>(i * hop + fft_size / 2) / sr;
        result.timestamps.push_back(time);

        if (std::isfinite(vocal_db)) vocal_sum += vocal_lin;
        total_sum += total_lin;
    }

    result.vocal_presence_score = (total_sum > 0) ?
        static_cast<float>(vocal_sum / total_sum) : 0.0f;
    result.masking_risk_score = static_cast<float>(masked_frames) / static_cast<float>(n);

    return result;
}

} // namespace evensteven
