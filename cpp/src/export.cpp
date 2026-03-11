#include "evensteven/export.h"
#include "evensteven/logging.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace evensteven {

using json = nlohmann::json;

static json loudness_to_json(const LoudnessResult& l, bool include_timelines) {
    json j;
    j["integrated_lufs"] = std::isfinite(l.integrated_lufs) ? json(l.integrated_lufs) : json(nullptr);
    j["rms_db"] = std::isfinite(l.rms_db) ? json(l.rms_db) : json(nullptr);
    j["peak_db"] = std::isfinite(l.peak_db) ? json(l.peak_db) : json(nullptr);
    j["dynamic_range_db"] = l.dynamic_range_db;

    json bands;
    for (auto& [name, val] : l.band_energy) {
        bands[name] = std::isfinite(val) ? json(val) : json(nullptr);
    }
    j["band_energy"] = bands;

    if (include_timelines) {
        j["shortterm_lufs"] = l.shortterm_lufs;
        j["momentary_lufs"] = l.momentary_lufs;
        j["timestamps"] = l.timestamps;
    }

    return j;
}

static json balance_to_json(const BalanceResult& b, bool include_timelines) {
    json j;
    j["vocal_presence_score"] = b.vocal_presence_score;
    j["masking_risk_score"] = b.masking_risk_score;

    if (include_timelines) {
        j["vocal_energy_db"] = b.vocal_energy_db;
        j["accompaniment_energy_db"] = b.accompaniment_energy_db;
        j["balance_db"] = b.balance_db;
        j["masking_risk"] = b.masking_risk;
        j["timestamps"] = b.timestamps;
    }

    return j;
}

static json section_to_json(const Section& s) {
    return {
        {"label", s.label},
        {"start_s", s.start_s},
        {"end_s", s.end_s},
        {"is_silence", s.is_silence}
    };
}

static json song_score_to_json(const SongScore& s) {
    return {
        {"loudness_consistency", s.loudness_consistency},
        {"balance_score", s.balance_score},
        {"composite", s.composite},
        {"grade", s.grade}
    };
}

static json setlist_to_full_json(const SetlistAnalysis& setlist) {
    bool timelines = setlist.config.export_cfg.include_timelines;

    json songs_arr = json::array();
    for (auto& song : setlist.songs) {
        json sj;
        sj["song_id"] = song.song_id;
        sj["song_name"] = song.song_name;
        sj["metadata"] = {
            {"filename", song.metadata.filename},
            {"duration_s", song.metadata.duration_s},
            {"sample_rate", song.metadata.sample_rate},
            {"channels", song.metadata.channels},
            {"format", song.metadata.format}
        };
        sj["loudness"] = loudness_to_json(song.loudness, timelines);
        sj["balance"] = balance_to_json(song.balance, timelines);

        json sections_arr = json::array();
        for (auto& sec : song.sections) {
            sections_arr.push_back(section_to_json(sec));
        }
        sj["sections"] = sections_arr;
        sj["scores"] = song_score_to_json(song.scores);
        sj["recommendations"] = song.recommendations;

        songs_arr.push_back(sj);
    }

    json result;
    result["project_id"] = setlist.project_id;
    result["project_name"] = setlist.project_name;
    result["timestamp"] = setlist.timestamp;
    result["setlist"] = {
        {"median_lufs", setlist.setlist_score.median_lufs},
        {"lufs_stddev", setlist.setlist_score.lufs_stddev},
        {"consistency_score", setlist.setlist_score.consistency_score},
        {"grade", setlist.setlist_score.grade},
        {"recommendations", setlist.setlist_recommendations}
    };
    result["songs"] = songs_arr;

    return result;
}

std::string to_json_string(const SetlistAnalysis& setlist) {
    return setlist_to_full_json(setlist).dump(2);
}

static void write_csv(const std::string& path, const std::vector<std::vector<std::string>>& rows) {
    std::ofstream f(path);
    if (!f.is_open()) {
        throw std::runtime_error("Cannot write CSV: " + path);
    }
    for (auto& row : rows) {
        for (size_t i = 0; i < row.size(); i++) {
            if (i > 0) f << ",";
            // Simple CSV quoting
            if (row[i].find(',') != std::string::npos || row[i].find('"') != std::string::npos) {
                f << "\"";
                for (char c : row[i]) {
                    if (c == '"') f << "\"\"";
                    else f << c;
                }
                f << "\"";
            } else {
                f << row[i];
            }
        }
        f << "\n";
    }
}

static std::string fmt_float(float v) {
    if (!std::isfinite(v)) return "";
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2) << v;
    return ss.str();
}

void export_analysis(const SetlistAnalysis& setlist, const std::string& output_dir, const Config& config) {
    namespace fs = std::filesystem;
    fs::create_directories(output_dir);

    // analysis.json
    {
        std::ofstream f(fs::path(output_dir) / "analysis.json");
        f << setlist_to_full_json(setlist).dump(2);
        log(LogLevel::INFO, "Wrote analysis.json");
    }

    // config.json
    {
        std::ifstream default_cfg("config/default_config.json");
        if (default_cfg.is_open()) {
            std::ofstream f(fs::path(output_dir) / "config.json");
            f << default_cfg.rdbuf();
        }
    }

    bool write_csv_files = (config.export_cfg.format == "csv" || config.export_cfg.format == "both");
    bool write_json = (config.export_cfg.format == "json" || config.export_cfg.format == "both");

    if (write_csv_files) {
        // song_summary.csv
        {
            std::vector<std::vector<std::string>> rows;
            rows.push_back({"song_id", "song_name", "duration_s", "integrated_lufs",
                           "rms_db", "peak_db", "dynamic_range_db",
                           "vocal_presence", "masking_risk", "composite_score", "grade"});
            for (auto& song : setlist.songs) {
                rows.push_back({
                    song.song_id, song.song_name,
                    fmt_float(song.metadata.duration_s),
                    fmt_float(song.loudness.integrated_lufs),
                    fmt_float(song.loudness.rms_db),
                    fmt_float(song.loudness.peak_db),
                    fmt_float(song.loudness.dynamic_range_db),
                    fmt_float(song.balance.vocal_presence_score),
                    fmt_float(song.balance.masking_risk_score),
                    fmt_float(song.scores.composite),
                    song.scores.grade
                });
            }
            write_csv((fs::path(output_dir) / "song_summary.csv").string(), rows);
            log(LogLevel::INFO, "Wrote song_summary.csv");
        }

        // section_summary.csv
        {
            std::vector<std::vector<std::string>> rows;
            rows.push_back({"song_id", "section", "start_s", "end_s", "is_silence"});
            for (auto& song : setlist.songs) {
                for (auto& sec : song.sections) {
                    rows.push_back({
                        song.song_id, sec.label,
                        fmt_float(sec.start_s), fmt_float(sec.end_s),
                        sec.is_silence ? "true" : "false"
                    });
                }
            }
            write_csv((fs::path(output_dir) / "section_summary.csv").string(), rows);
            log(LogLevel::INFO, "Wrote section_summary.csv");
        }

        // Per-song timelines
        if (config.export_cfg.include_timelines) {
            for (auto& song : setlist.songs) {
                // Loudness timeline
                {
                    std::vector<std::vector<std::string>> rows;
                    rows.push_back({"time_s", "momentary_lufs", "shortterm_lufs"});
                    for (size_t i = 0; i < song.loudness.timestamps.size(); i++) {
                        rows.push_back({
                            fmt_float(song.loudness.timestamps[i]),
                            i < song.loudness.momentary_lufs.size() ? fmt_float(song.loudness.momentary_lufs[i]) : "",
                            i < song.loudness.shortterm_lufs.size() ? fmt_float(song.loudness.shortterm_lufs[i]) : ""
                        });
                    }
                    write_csv((fs::path(output_dir) / ("loudness_timeline_" + song.song_id + ".csv")).string(), rows);
                }

                // Balance timeline
                {
                    std::vector<std::vector<std::string>> rows;
                    rows.push_back({"time_s", "vocal_energy_db", "accompaniment_energy_db", "balance_db", "masking_risk"});
                    for (size_t i = 0; i < song.balance.timestamps.size(); i++) {
                        rows.push_back({
                            fmt_float(song.balance.timestamps[i]),
                            i < song.balance.vocal_energy_db.size() ? fmt_float(song.balance.vocal_energy_db[i]) : "",
                            i < song.balance.accompaniment_energy_db.size() ? fmt_float(song.balance.accompaniment_energy_db[i]) : "",
                            i < song.balance.balance_db.size() ? fmt_float(song.balance.balance_db[i]) : "",
                            i < song.balance.masking_risk.size() ? (song.balance.masking_risk[i] ? "true" : "false") : ""
                        });
                    }
                    write_csv((fs::path(output_dir) / ("balance_timeline_" + song.song_id + ".csv")).string(), rows);
                }
            }
            log(LogLevel::INFO, "Wrote timeline CSVs");
        }
    }

    log(LogLevel::INFO, "Export complete to " + output_dir);
}

} // namespace evensteven
