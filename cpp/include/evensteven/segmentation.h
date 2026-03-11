#pragma once

#include <vector>
#include <string>
#include "audio_io.h"
#include "config.h"

namespace evensteven {

struct Section {
    std::string label;
    float start_s = 0.0f;
    float end_s = 0.0f;
    bool is_silence = false;
};

std::vector<Section> auto_segment(const AudioBuffer& audio, const Config& config);
AudioBuffer extract_section(const AudioBuffer& audio, const Section& section);
std::vector<Section> merge_sections(const std::vector<Section>& user_sections,
                                     const std::vector<Section>& auto_sections);

} // namespace evensteven
