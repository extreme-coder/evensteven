#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "evensteven/segmentation.h"
#include <cmath>

using Catch::Matchers::WithinAbs;

static evensteven::AudioBuffer make_audio_with_silence(int sr = 48000) {
    // 3 seconds tone, 2 seconds silence, 3 seconds tone
    evensteven::AudioBuffer buf;
    buf.sample_rate = sr;
    buf.channels = 1;
    int total = 8 * sr;
    buf.samples.resize(total);

    for (int i = 0; i < total; i++) {
        float t = static_cast<float>(i) / sr;
        if (t < 3.0f || t >= 5.0f) {
            buf.samples[i] = 0.5f * std::sin(2.0f * static_cast<float>(M_PI) * 440.0f * i / sr);
        } else {
            buf.samples[i] = 0.0f;
        }
    }
    return buf;
}

TEST_CASE("auto_segment detects silence boundaries", "[segmentation]") {
    auto audio = make_audio_with_silence();
    auto config = evensteven::Config::default_config();
    auto sections = evensteven::auto_segment(audio, config);

    // Should have at least 2 non-silence sections
    REQUIRE(sections.size() >= 2);

    // First section should start at 0
    REQUIRE_THAT(static_cast<double>(sections.front().start_s), WithinAbs(0.0, 0.5));

    // Last section should end near 8s
    REQUIRE_THAT(static_cast<double>(sections.back().end_s), WithinAbs(8.0, 0.5));
}

TEST_CASE("extract_section extracts correct range", "[segmentation]") {
    evensteven::AudioBuffer buf;
    buf.sample_rate = 100;
    buf.channels = 1;
    buf.samples.resize(1000);
    for (int i = 0; i < 1000; i++) buf.samples[i] = static_cast<float>(i);

    evensteven::Section sec;
    sec.start_s = 2.0f;
    sec.end_s = 5.0f;

    auto extracted = evensteven::extract_section(buf, sec);
    REQUIRE(extracted.sample_rate == 100);
    REQUIRE(extracted.num_frames() == 300);
    REQUIRE_THAT(static_cast<double>(extracted.samples[0]), WithinAbs(200.0, 1.0));
}

TEST_CASE("auto_segment handles short audio", "[segmentation]") {
    evensteven::AudioBuffer buf;
    buf.sample_rate = 48000;
    buf.channels = 1;
    buf.samples.resize(48000); // 1 second
    for (auto& s : buf.samples) s = 0.5f;

    auto config = evensteven::Config::default_config();
    auto sections = evensteven::auto_segment(buf, config);

    REQUIRE(!sections.empty());
    REQUIRE(sections[0].label == "section_1");
}
