#include <catch2/catch_test_macros.hpp>
#include "evensteven/export.h"
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>

using json = nlohmann::json;

static evensteven::SetlistAnalysis make_test_setlist() {
    evensteven::SetlistAnalysis setlist;
    setlist.project_id = "test_proj";
    setlist.project_name = "Test Project";
    setlist.timestamp = "2026-01-01T00:00:00";
    setlist.config = evensteven::Config::default_config();

    evensteven::SongAnalysis song;
    song.song_id = "test_song";
    song.song_name = "Test Song";
    song.metadata.filename = "test.wav";
    song.metadata.duration_s = 30.0f;
    song.metadata.sample_rate = 48000;
    song.metadata.channels = 2;
    song.metadata.format = "wav";
    song.loudness.integrated_lufs = -14.0f;
    song.loudness.rms_db = -16.0f;
    song.loudness.peak_db = -1.0f;
    song.loudness.dynamic_range_db = 8.0f;
    song.loudness.band_energy["low"] = -20.0f;
    song.loudness.band_energy["mid"] = -14.0f;
    song.loudness.band_energy["high"] = -22.0f;
    song.loudness.shortterm_lufs = {-14.0f, -13.5f, -14.2f};
    song.loudness.momentary_lufs = {-14.0f, -13.5f, -14.2f};
    song.loudness.timestamps = {0.5f, 1.0f, 1.5f};
    song.balance.vocal_presence_score = 0.6f;
    song.balance.masking_risk_score = 0.1f;
    song.balance.vocal_energy_db = {-15.0f, -14.0f};
    song.balance.accompaniment_energy_db = {-18.0f, -17.0f};
    song.balance.balance_db = {3.0f, 3.0f};
    song.balance.masking_risk = {false, false};
    song.balance.timestamps = {0.5f, 1.0f};
    song.sections = {{"intro", 0.0f, 10.0f, false}, {"verse", 10.0f, 30.0f, false}};
    song.scores = {0.8f, 0.9f, 0.85f, "Good"};
    song.recommendations = {"Good balance."};

    setlist.songs.push_back(song);
    setlist.setlist_score = {0.9f, 0.5f, -14.0f, "Good"};
    setlist.setlist_recommendations = {"Well balanced."};

    return setlist;
}

TEST_CASE("to_json_string produces valid JSON", "[export]") {
    auto setlist = make_test_setlist();
    auto json_str = evensteven::to_json_string(setlist);

    auto j = json::parse(json_str);
    REQUIRE(j["project_id"] == "test_proj");
    REQUIRE(j["songs"].size() == 1);
    REQUIRE(j["songs"][0]["song_id"] == "test_song");
    REQUIRE(j["songs"][0]["loudness"]["integrated_lufs"].get<float>() == -14.0f);
}

TEST_CASE("export_analysis writes files", "[export]") {
    auto setlist = make_test_setlist();
    std::string dir = "/tmp/test_evensteven_export";
    std::filesystem::remove_all(dir);

    evensteven::export_analysis(setlist, dir, setlist.config);

    REQUIRE(std::filesystem::exists(dir + "/analysis.json"));
    REQUIRE(std::filesystem::exists(dir + "/song_summary.csv"));
    REQUIRE(std::filesystem::exists(dir + "/section_summary.csv"));

    // Validate JSON
    std::ifstream f(dir + "/analysis.json");
    auto j = json::parse(f);
    REQUIRE(j["songs"].size() == 1);

    std::filesystem::remove_all(dir);
}
