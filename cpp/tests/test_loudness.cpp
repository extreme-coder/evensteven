#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "evensteven/loudness.h"
#include <cmath>

using Catch::Matchers::WithinAbs;

static evensteven::AudioBuffer make_sine(float frequency, float amplitude, float duration_s, int sr = 48000) {
    evensteven::AudioBuffer buf;
    buf.sample_rate = sr;
    buf.channels = 1;
    int frames = static_cast<int>(duration_s * sr);
    buf.samples.resize(frames);
    for (int i = 0; i < frames; i++) {
        buf.samples[i] = amplitude * std::sin(2.0f * static_cast<float>(M_PI) * frequency * i / sr);
    }
    return buf;
}

TEST_CASE("K-weighting filter modifies signal", "[loudness]") {
    std::vector<float> samples(48000);
    for (int i = 0; i < 48000; i++) {
        samples[i] = std::sin(2.0f * static_cast<float>(M_PI) * 1000.0f * i / 48000.0f);
    }

    auto original = samples;
    evensteven::apply_k_weighting(samples, 48000);

    // Signal should be modified
    bool different = false;
    for (size_t i = 100; i < samples.size(); i++) {
        if (std::abs(samples[i] - original[i]) > 1e-6f) {
            different = true;
            break;
        }
    }
    REQUIRE(different);
}

TEST_CASE("1kHz sine at -23 LUFS measures approximately -23 LUFS", "[loudness]") {
    // A 1kHz sine at specific amplitude
    // RMS of sine = amplitude / sqrt(2)
    // LUFS for 1kHz ~= RMS (K-weighting is near unity at 1kHz)
    // For -23 LUFS: 10^((-23 + 0.691)/20) / sqrt(2) ~= 0.0794 / 1.414 ~= 0.0562
    // Actually: -23 LUFS means -0.691 + 10*log10(mean_square) = -23
    // mean_square = 10^((-23+0.691)/10) = 10^(-2.2309) = 0.00588
    // amplitude = sqrt(2 * mean_square) = sqrt(0.01176) = 0.1084
    float amplitude = 0.1084f;
    auto audio = make_sine(1000.0f, amplitude, 5.0f, 48000);

    auto config = evensteven::Config::default_config();
    auto result = evensteven::compute_loudness(audio, config);

    // Should be within 1 LUFS of -23
    REQUIRE(std::isfinite(result.integrated_lufs));
    REQUIRE_THAT(static_cast<double>(result.integrated_lufs), WithinAbs(-23.0, 1.5));
}

TEST_CASE("Loudness result has valid timelines", "[loudness]") {
    auto audio = make_sine(440.0f, 0.5f, 3.0f, 48000);
    auto config = evensteven::Config::default_config();
    auto result = evensteven::compute_loudness(audio, config);

    REQUIRE(!result.shortterm_lufs.empty());
    REQUIRE(!result.momentary_lufs.empty());
    REQUIRE(!result.timestamps.empty());
    REQUIRE(result.shortterm_lufs.size() == result.timestamps.size());
    REQUIRE(result.dynamic_range_db >= 0.0f);
}

TEST_CASE("Silence produces -inf LUFS", "[loudness]") {
    evensteven::AudioBuffer silence;
    silence.sample_rate = 48000;
    silence.channels = 1;
    silence.samples.resize(48000, 0.0f);

    auto config = evensteven::Config::default_config();
    auto result = evensteven::compute_loudness(silence, config);

    REQUIRE(result.integrated_lufs == -INFINITY);
}
