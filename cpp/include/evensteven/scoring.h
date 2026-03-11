#pragma once

#include <vector>
#include <string>
#include "loudness.h"
#include "balance.h"
#include "segmentation.h"
#include "config.h"

namespace evensteven {

struct SongScore {
    float loudness_consistency = 0.0f;
    float balance_score = 0.0f;
    float composite = 0.0f;
    std::string grade;
};

struct SetlistScore {
    float consistency_score = 0.0f;
    float lufs_stddev = 0.0f;
    float median_lufs = 0.0f;
    std::string grade;
};

struct TakeComparison {
    float lufs_difference = 0.0f;
    float balance_difference = 0.0f;
    std::string recommendation;
};

struct SongAnalysis {
    std::string song_id;
    std::string song_name;
    AudioMetadata metadata;
    LoudnessResult loudness;
    BalanceResult balance;
    std::vector<Section> sections;
    SongScore scores;
    std::vector<std::string> recommendations;
};

SongScore score_song(const LoudnessResult& loudness, const BalanceResult& balance, const Config& config);
SetlistScore score_setlist(const std::vector<SongAnalysis>& songs, const Config& config);
std::vector<std::string> generate_recommendations(const SongAnalysis& song,
                                                    const SetlistScore& setlist,
                                                    const Config& config);
TakeComparison compare_takes(const SongAnalysis& take1, const SongAnalysis& take2);

} // namespace evensteven
