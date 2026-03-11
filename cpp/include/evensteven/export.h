#pragma once

#include <string>
#include <vector>
#include "scoring.h"
#include "config.h"

namespace evensteven {

struct SetlistAnalysis {
    std::string project_id;
    std::string project_name;
    std::string timestamp;
    Config config;
    SetlistScore setlist_score;
    std::vector<std::string> setlist_recommendations;
    std::vector<SongAnalysis> songs;
};

void export_analysis(const SetlistAnalysis& setlist, const std::string& output_dir, const Config& config);
std::string to_json_string(const SetlistAnalysis& setlist);

} // namespace evensteven
