#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "evensteven/preprocessing.h"
#include <cmath>

using Catch::Matchers::WithinAbs;

static evensteven::AudioBuffer make_stereo_buffer(int frames, int sr = 44100) {
    evensteven::AudioBuffer buf;
    buf.sample_rate = sr;
    buf.channels = 2;
    buf.samples.resize(frames * 2);
    for (int i = 0; i < frames; i++) {
        float val = std::sin(2.0f * static_cast<float>(M_PI) * 440.0f * i / sr);
        buf.samples[i * 2] = val;
        buf.samples[i * 2 + 1] = val * 0.5f;
    }
    return buf;
}

TEST_CASE("to_mono averages channels", "[preprocessing]") {
    auto stereo = make_stereo_buffer(1000);
    auto mono = evensteven::to_mono(stereo);

    REQUIRE(mono.channels == 1);
    REQUIRE(mono.num_frames() == 1000);

    // Each mono sample = average of L and R
    for (int i = 0; i < 10; i++) {
        float expected = (stereo.samples[i * 2] + stereo.samples[i * 2 + 1]) / 2.0f;
        REQUIRE_THAT(mono.samples[i], WithinAbs(expected, 1e-6f));
    }
}

TEST_CASE("to_mono is identity for mono input", "[preprocessing]") {
    evensteven::AudioBuffer mono;
    mono.sample_rate = 44100;
    mono.channels = 1;
    mono.samples = {0.1f, 0.2f, 0.3f};

    auto result = evensteven::to_mono(mono);
    REQUIRE(result.samples == mono.samples);
}

TEST_CASE("normalize_peak scales to target", "[preprocessing]") {
    evensteven::AudioBuffer buf;
    buf.sample_rate = 44100;
    buf.channels = 1;
    buf.samples = {0.5f, -0.25f, 0.1f};

    auto norm = evensteven::normalize_peak(buf, 1.0f);
    REQUIRE_THAT(norm.samples[0], WithinAbs(1.0f, 1e-6f));
    REQUIRE_THAT(norm.samples[1], WithinAbs(-0.5f, 1e-6f));
}

TEST_CASE("resample changes sample rate", "[preprocessing]") {
    evensteven::AudioBuffer buf;
    buf.sample_rate = 44100;
    buf.channels = 1;
    buf.samples.resize(44100);
    for (int i = 0; i < 44100; i++) {
        buf.samples[i] = std::sin(2.0f * static_cast<float>(M_PI) * 440.0f * i / 44100.0f);
    }

    auto resampled = evensteven::resample(buf, 48000);
    REQUIRE(resampled.sample_rate == 48000);
    REQUIRE(resampled.channels == 1);
    // Duration should be approximately the same
    REQUIRE_THAT(resampled.duration_s(), WithinAbs(1.0f, 0.02f));
}
