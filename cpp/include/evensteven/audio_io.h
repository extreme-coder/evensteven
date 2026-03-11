#pragma once

#include <string>
#include <vector>

namespace evensteven {

struct AudioBuffer {
    std::vector<float> samples;
    int sample_rate = 0;
    int channels = 0;

    int num_frames() const;
    float duration_s() const;
};

struct AudioMetadata {
    std::string filename;
    float duration_s = 0.0f;
    int sample_rate = 0;
    int channels = 0;
    std::string format;
};

AudioBuffer load_audio(const std::string& filepath);
AudioMetadata probe_audio(const std::string& filepath);
bool is_supported_format(const std::string& filepath);

} // namespace evensteven
