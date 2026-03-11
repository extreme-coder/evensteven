#include "evensteven/scoring.h"
#include <cmath>
#include <algorithm>
#include <numeric>

namespace evensteven {

static std::string grade_from_score(float score, const Config& config) {
    if (score >= config.scoring.good_threshold) return "Good";
    if (score >= config.scoring.fair_threshold) return "Fair";
    return "Needs Attention";
}

SongScore score_song(const LoudnessResult& loudness, const BalanceResult& balance, const Config& config) {
    SongScore score;

    // Loudness consistency: based on dynamic range relative to 12 dB reference
    float dr = std::max(0.0f, loudness.dynamic_range_db);
    score.loudness_consistency = std::max(0.0f, 1.0f - dr / 12.0f);

    // Balance score: inverse of masking risk
    score.balance_score = 1.0f - balance.masking_risk_score;

    // Composite: weighted average
    score.composite = 0.5f * score.loudness_consistency + 0.5f * score.balance_score;
    score.grade = grade_from_score(score.composite, config);

    return score;
}

SetlistScore score_setlist(const std::vector<SongAnalysis>& songs, const Config& config) {
    SetlistScore score;

    if (songs.empty()) return score;

    std::vector<float> lufs_values;
    for (auto& song : songs) {
        if (std::isfinite(song.loudness.integrated_lufs)) {
            lufs_values.push_back(song.loudness.integrated_lufs);
        }
    }

    if (lufs_values.empty()) return score;

    // Median LUFS
    std::vector<float> sorted = lufs_values;
    std::sort(sorted.begin(), sorted.end());
    if (sorted.size() % 2 == 0) {
        score.median_lufs = (sorted[sorted.size() / 2 - 1] + sorted[sorted.size() / 2]) / 2.0f;
    } else {
        score.median_lufs = sorted[sorted.size() / 2];
    }

    // Standard deviation
    float mean = 0.0f;
    for (float v : lufs_values) mean += v;
    mean /= lufs_values.size();

    float var = 0.0f;
    for (float v : lufs_values) var += (v - mean) * (v - mean);
    score.lufs_stddev = std::sqrt(var / lufs_values.size());

    // Consistency score: 1.0 if stddev=0, decreasing with tolerance
    score.consistency_score = std::max(0.0f, 1.0f - score.lufs_stddev / config.scoring.lufs_tolerance);
    score.grade = grade_from_score(score.consistency_score, config);

    return score;
}

std::vector<std::string> generate_recommendations(const SongAnalysis& song,
                                                    const SetlistScore& setlist,
                                                    const Config& config) {
    std::vector<std::string> recs;

    // Check if song is significantly louder/quieter than setlist median
    if (std::isfinite(song.loudness.integrated_lufs) && std::isfinite(setlist.median_lufs)) {
        float diff = song.loudness.integrated_lufs - setlist.median_lufs;
        if (diff > config.scoring.lufs_tolerance) {
            recs.push_back("This song is significantly louder than the rest of the set (+" +
                          std::to_string(static_cast<int>(std::round(diff))) + " dB).");
        } else if (diff < -config.scoring.lufs_tolerance) {
            recs.push_back("This song is significantly quieter than the rest of the set (" +
                          std::to_string(static_cast<int>(std::round(diff))) + " dB).");
        }
    }

    // Masking risk
    float masking_pct = song.balance.masking_risk_score * 100.0f;
    if (masking_pct > 10.0f) {
        recs.push_back("Vocals may be masked in approximately " +
                       std::to_string(static_cast<int>(std::round(masking_pct))) + "% of this song.");
    }

    // Band energy dominance
    auto it_low = song.loudness.band_energy.find("low");
    auto it_mid = song.loudness.band_energy.find("mid");
    auto it_high = song.loudness.band_energy.find("high");
    if (it_low != song.loudness.band_energy.end() &&
        it_mid != song.loudness.band_energy.end()) {
        if (std::isfinite(it_low->second) && std::isfinite(it_mid->second) &&
            it_low->second > it_mid->second + 6.0f) {
            recs.push_back("Low-frequency energy dominates the mix.");
        }
    }

    // Section-specific masking
    for (auto& section : song.sections) {
        if (section.is_silence) continue;
        // Check masking in this section's time range
        int masked_in_section = 0;
        int total_in_section = 0;
        for (size_t i = 0; i < song.balance.timestamps.size(); i++) {
            float t = song.balance.timestamps[i];
            if (t >= section.start_s && t <= section.end_s) {
                total_in_section++;
                if (song.balance.masking_risk[i]) masked_in_section++;
            }
        }
        if (total_in_section > 0) {
            float section_masking = static_cast<float>(masked_in_section) / total_in_section;
            if (section_masking > 0.3f) {
                recs.push_back("Vocals are likely masked during " + section.label + ".");
            }
        }
    }

    if (recs.empty()) {
        recs.push_back("This song has a good balance and consistent loudness.");
    }

    return recs;
}

TakeComparison compare_takes(const SongAnalysis& take1, const SongAnalysis& take2) {
    TakeComparison cmp;
    cmp.lufs_difference = take2.loudness.integrated_lufs - take1.loudness.integrated_lufs;
    cmp.balance_difference = take2.balance.vocal_presence_score - take1.balance.vocal_presence_score;

    if (take2.scores.composite > take1.scores.composite) {
        cmp.recommendation = "Take 2 has a better overall score.";
    } else if (take1.scores.composite > take2.scores.composite) {
        cmp.recommendation = "Take 1 has a better overall score.";
    } else {
        cmp.recommendation = "Both takes are similar in quality.";
    }

    return cmp;
}

} // namespace evensteven
