#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "evensteven/balance.h"
#include <cmath>

using Catch::Matchers::WithinAbs;

static evensteven::AudioBuffer make_tone(float freq, float amplitude, float duration_s, int sr = 48000) {
    evensteven::AudioBuffer buf;
    buf.sample_rate = sr;
    buf.channels = 1;
    int frames = static_cast<int>(duration_s * sr);
    buf.samples.resize(frames);
    for (int i = 0; i < frames; i++) {
        buf.samples[i] = amplitude * std::sin(2.0f * static_cast<float>(M_PI) * freq * i / sr);
    }
    return buf;
}

static evensteven::AudioBuffer mix_buffers(const evensteven::AudioBuffer& a, const evensteven::AudioBuffer& b) {
    evensteven::AudioBuffer out = a;
    for (size_t i = 0; i < std::min(a.samples.size(), b.samples.size()); i++) {
        out.samples[i] += b.samples[i];
    }
    return out;
}

TEST_CASE("Balance detects vocal-range dominant signal", "[balance]") {
    // 1kHz tone (vocal range) at high amplitude
    auto vocal = make_tone(1000.0f, 0.8f, 2.0f);
    // 100Hz tone (bass) at low amplitude
    auto bass = make_tone(100.0f, 0.1f, 2.0f);
    auto mixed = mix_buffers(vocal, bass);

    auto config = evensteven::Config::default_config();
    auto result = evensteven::compute_balance(mixed, config);

    // Vocal presence should be high
    REQUIRE(result.vocal_presence_score > 0.5f);
    // Low masking risk since vocals dominate
    REQUIRE(result.masking_risk_score < 0.5f);
}

TEST_CASE("Balance detects masking when bass dominates", "[balance]") {
    // 1kHz tone (vocal range) at low amplitude
    auto vocal = make_tone(1000.0f, 0.05f, 2.0f);
    // 100Hz + 200Hz (bass) at high amplitude
    auto bass = make_tone(100.0f, 0.8f, 2.0f);
    auto mixed = mix_buffers(vocal, bass);

    auto config = evensteven::Config::default_config();
    auto result = evensteven::compute_balance(mixed, config);

    // Masking risk should be higher
    REQUIRE(result.masking_risk_score > 0.0f);
}

TEST_CASE("Balance result has valid timelines", "[balance]") {
    auto audio = make_tone(440.0f, 0.5f, 2.0f);
    auto config = evensteven::Config::default_config();
    auto result = evensteven::compute_balance(audio, config);

    REQUIRE(!result.timestamps.empty());
    REQUIRE(result.vocal_energy_db.size() == result.timestamps.size());
    REQUIRE(result.accompaniment_energy_db.size() == result.timestamps.size());
    REQUIRE(result.balance_db.size() == result.timestamps.size());
    REQUIRE(result.masking_risk.size() == result.timestamps.size());
}
