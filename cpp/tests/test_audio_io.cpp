#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "evensteven/audio_io.h"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <vector>
#include <cstring>

using Catch::Matchers::WithinAbs;

// Helper: write a minimal WAV file with sine wave
static void write_test_wav(const std::string& path, int sample_rate, int channels,
                           int num_frames, float frequency = 440.0f) {
    std::vector<float> samples(num_frames * channels);
    for (int i = 0; i < num_frames; i++) {
        float val = std::sin(2.0f * static_cast<float>(M_PI) * frequency * i / sample_rate);
        for (int ch = 0; ch < channels; ch++) {
            samples[i * channels + ch] = val;
        }
    }

    // Write raw WAV
    std::ofstream f(path, std::ios::binary);
    int data_size = num_frames * channels * sizeof(int16_t);
    int file_size = 36 + data_size;

    f.write("RIFF", 4);
    f.write(reinterpret_cast<char*>(&file_size), 4);
    f.write("WAVE", 4);
    f.write("fmt ", 4);
    int fmt_size = 16;
    f.write(reinterpret_cast<char*>(&fmt_size), 4);
    int16_t audio_format = 1; // PCM
    f.write(reinterpret_cast<char*>(&audio_format), 2);
    int16_t ch_count = static_cast<int16_t>(channels);
    f.write(reinterpret_cast<char*>(&ch_count), 2);
    f.write(reinterpret_cast<char*>(&sample_rate), 4);
    int byte_rate = sample_rate * channels * 2;
    f.write(reinterpret_cast<char*>(&byte_rate), 4);
    int16_t block_align = static_cast<int16_t>(channels * 2);
    f.write(reinterpret_cast<char*>(&block_align), 2);
    int16_t bits = 16;
    f.write(reinterpret_cast<char*>(&bits), 2);
    f.write("data", 4);
    f.write(reinterpret_cast<char*>(&data_size), 4);

    for (int i = 0; i < num_frames * channels; i++) {
        int16_t s = static_cast<int16_t>(samples[i] * 32767.0f);
        f.write(reinterpret_cast<char*>(&s), 2);
    }
}

TEST_CASE("load_audio loads WAV file", "[audio_io]") {
    std::string path = (std::filesystem::temp_directory_path() / "test_evensteven_audio.wav").string();
    write_test_wav(path, 44100, 2, 44100); // 1 second stereo

    auto buf = evensteven::load_audio(path);
    REQUIRE(buf.sample_rate == 44100);
    REQUIRE(buf.channels == 2);
    REQUIRE(buf.num_frames() == 44100);
    REQUIRE_THAT(buf.duration_s(), WithinAbs(1.0f, 0.01f));
}

TEST_CASE("probe_audio reads metadata", "[audio_io]") {
    std::string path = (std::filesystem::temp_directory_path() / "test_evensteven_probe.wav").string();
    write_test_wav(path, 48000, 1, 48000);

    auto meta = evensteven::probe_audio(path);
    REQUIRE(meta.sample_rate == 48000);
    REQUIRE(meta.channels == 1);
    REQUIRE_THAT(meta.duration_s, WithinAbs(1.0f, 0.01f));
    REQUIRE(meta.format == "wav");
}

TEST_CASE("is_supported_format", "[audio_io]") {
    REQUIRE(evensteven::is_supported_format("test.wav"));
    REQUIRE(evensteven::is_supported_format("test.WAV"));
    REQUIRE(evensteven::is_supported_format("test.flac"));
    REQUIRE(evensteven::is_supported_format("test.mp3"));
    REQUIRE_FALSE(evensteven::is_supported_format("test.ogg"));
    REQUIRE_FALSE(evensteven::is_supported_format("test.txt"));
}

TEST_CASE("load_audio throws on bad file", "[audio_io]") {
    REQUIRE_THROWS(evensteven::load_audio((std::filesystem::temp_directory_path() / "nonexistent_file.wav").string()));
}
