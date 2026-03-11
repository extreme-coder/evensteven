#include "evensteven/config.h"
#include "evensteven/logging.h"
#include "evensteven/audio_io.h"
#include "evensteven/preprocessing.h"
#include "evensteven/segmentation.h"
#include "evensteven/loudness.h"
#include "evensteven/balance.h"
#include "evensteven/scoring.h"
#include "evensteven/export.h"
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <filesystem>
#include <algorithm>

using namespace evensteven;

struct CliArgs {
    std::string config_path;
    std::string output_dir = "./output";
    std::string project_name = "Untitled Project";
    std::string format = "both";
    std::vector<std::string> files;
    bool demo = false;
    bool probe = false;
    bool progress = false;
    bool verbose = false;
    bool help = false;
    bool version = false;
};

static void print_help() {
    std::cerr << "EvenSteven - Setlist Loudness Balancer\n\n"
              << "Usage: evensteven [options] <audio_files...>\n\n"
              << "Options:\n"
              << "  --config <path>    Config JSON (default: built-in)\n"
              << "  --output <dir>     Output directory (default: ./output)\n"
              << "  --project <name>   Project name\n"
              << "  --format json|csv|both\n"
              << "  --demo             Use bundled demo audio\n"
              << "  --progress         Stream progress JSON to stdout\n"
              << "  --verbose          Verbose logging\n"
              << "  --help             Show this help\n"
              << "  --version          Show version\n";
}

static void print_version() {
    std::cerr << "evensteven 1.0.0\n";
}

static CliArgs parse_args(int argc, char* argv[]) {
    CliArgs args;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--config" && i + 1 < argc) {
            args.config_path = argv[++i];
        } else if (arg == "--output" && i + 1 < argc) {
            args.output_dir = argv[++i];
        } else if (arg == "--project" && i + 1 < argc) {
            args.project_name = argv[++i];
        } else if (arg == "--format" && i + 1 < argc) {
            args.format = argv[++i];
        } else if (arg == "--demo") {
            args.demo = true;
        } else if (arg == "--probe") {
            args.probe = true;
        } else if (arg == "--progress") {
            args.progress = true;
        } else if (arg == "--verbose") {
            args.verbose = true;
        } else if (arg == "--help" || arg == "-h") {
            args.help = true;
        } else if (arg == "--version" || arg == "-v") {
            args.version = true;
        } else if (arg[0] != '-') {
            args.files.push_back(arg);
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
        }
    }
    return args;
}

static std::string generate_project_id() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return "proj_" + std::to_string(ms);
}

static std::string generate_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tm);
    return buf;
}

static std::string song_id_from_path(const std::string& path) {
    std::string stem = std::filesystem::path(path).stem().string();
    std::replace(stem.begin(), stem.end(), ' ', '_');
    std::transform(stem.begin(), stem.end(), stem.begin(), ::tolower);
    return stem;
}

static void emit_error(const std::string& msg) {
    std::cout << "{\"status\":\"error\",\"message\":\"" << msg << "\"}" << std::endl;
}

int main(int argc, char* argv[]) {
    CliArgs args = parse_args(argc, argv);

    if (args.help) { print_help(); return 0; }
    if (args.version) { print_version(); return 0; }

    set_verbose(args.verbose);

    // Load config
    Config config;
    if (!args.config_path.empty()) {
        try {
            config = Config::load_from_file(args.config_path);
        } catch (const std::exception& e) {
            emit_error(std::string("Config error: ") + e.what());
            return 1;
        }
    } else {
        config = Config::default_config();
    }
    config.export_cfg.format = args.format;

    // Resolve demo files
    if (args.demo) {
        namespace fs = std::filesystem;
        // Look for demo files relative to executable
        std::string exe_dir = fs::path(argv[0]).parent_path().string();
        std::vector<std::string> search_paths = {
            "examples/audio",
            exe_dir + "/../examples/audio",
            exe_dir + "/../../examples/audio"
        };
        for (auto& dir : search_paths) {
            if (fs::exists(dir)) {
                for (auto& entry : fs::directory_iterator(dir)) {
                    if (is_supported_format(entry.path().string())) {
                        args.files.push_back(entry.path().string());
                    }
                }
                if (!args.files.empty()) break;
            }
        }
        if (args.files.empty()) {
            emit_error("No demo audio files found");
            return 1;
        }
        std::sort(args.files.begin(), args.files.end());
        args.project_name = "Demo Setlist";
    }

    if (args.files.empty()) {
        std::cerr << "No input files specified. Use --help for usage.\n";
        return 1;
    }

    // Validate files
    for (auto& f : args.files) {
        if (!std::filesystem::exists(f)) {
            emit_error("File not found: " + f);
            return 1;
        }
        if (!is_supported_format(f)) {
            emit_error("Unsupported format: " + f);
            return 1;
        }
    }

    // Probe-only mode: output metadata JSON and exit
    if (args.probe) {
        std::cout << "[";
        for (size_t i = 0; i < args.files.size(); i++) {
            try {
                auto meta = probe_audio(args.files[i]);
                if (i > 0) std::cout << ",";
                std::cout << "{\"filename\":\"" << meta.filename
                          << "\",\"duration_s\":" << meta.duration_s
                          << ",\"sample_rate\":" << meta.sample_rate
                          << ",\"channels\":" << meta.channels
                          << ",\"format\":\"" << meta.format << "\"}";
            } catch (const std::exception& e) {
                if (i > 0) std::cout << ",";
                std::cout << "{\"filename\":\"" << std::filesystem::path(args.files[i]).filename().string()
                          << "\",\"error\":\"" << e.what() << "\"}";
            }
        }
        std::cout << "]" << std::endl;
        return 0;
    }

    // Analyze each song
    SetlistAnalysis setlist;
    setlist.project_id = generate_project_id();
    setlist.project_name = args.project_name;
    setlist.timestamp = generate_timestamp();
    setlist.config = config;

    int total_songs = static_cast<int>(args.files.size());
    for (int i = 0; i < total_songs; i++) {
        auto& filepath = args.files[i];
        std::string name = std::filesystem::path(filepath).stem().string();
        float base_pct = 100.0f * i / total_songs;
        float song_pct = 100.0f / total_songs;

        if (args.progress) {
            report_progress("loading", base_pct, name);
        }

        try {
            SongAnalysis song;
            song.song_id = song_id_from_path(filepath);
            song.song_name = name;
            song.metadata = probe_audio(filepath);

            log(LogLevel::INFO, "Processing: " + name);

            // Load audio
            AudioBuffer audio = load_audio(filepath);

            if (args.progress) {
                report_progress("preprocessing", base_pct + song_pct * 0.1f, name);
            }

            // Preprocess
            AudioBuffer mono = to_mono(audio);
            if (mono.sample_rate != 48000 && mono.sample_rate != 44100) {
                mono = resample(mono, 48000);
            }

            if (args.progress) {
                report_progress("segmentation", base_pct + song_pct * 0.2f, name);
            }

            // Segment
            song.sections = auto_segment(audio, config);

            if (args.progress) {
                report_progress("loudness", base_pct + song_pct * 0.4f, name);
            }

            // Loudness
            song.loudness = compute_loudness(audio, config);

            if (args.progress) {
                report_progress("balance", base_pct + song_pct * 0.7f, name);
            }

            // Balance
            song.balance = compute_balance(audio, config);

            if (args.progress) {
                report_progress("scoring", base_pct + song_pct * 0.9f, name);
            }

            // Score
            song.scores = score_song(song.loudness, song.balance, config);

            setlist.songs.push_back(std::move(song));

        } catch (const std::exception& e) {
            log(LogLevel::ERROR, "Error processing " + name + ": " + e.what());
            emit_error(std::string("Error processing ") + name + ": " + e.what());
            return 1;
        }
    }

    // Setlist scoring
    if (args.progress) {
        report_progress("setlist_scoring", 95.0f, "");
    }

    setlist.setlist_score = score_setlist(setlist.songs, config);

    // Generate recommendations
    for (auto& song : setlist.songs) {
        song.recommendations = generate_recommendations(song, setlist.setlist_score, config);
    }

    // Setlist-level recommendations
    if (setlist.setlist_score.lufs_stddev > config.scoring.lufs_tolerance) {
        setlist.setlist_recommendations.push_back(
            "Song volumes vary significantly across the set. Consider adjusting levels.");
    }
    if (setlist.setlist_score.consistency_score >= config.scoring.good_threshold) {
        setlist.setlist_recommendations.push_back(
            "Overall setlist loudness is well-balanced.");
    }

    // Export
    if (args.progress) {
        report_progress("exporting", 98.0f, "");
    }

    try {
        export_analysis(setlist, args.output_dir, config);
    } catch (const std::exception& e) {
        emit_error(std::string("Export error: ") + e.what());
        return 1;
    }

    // Print summary
    log(LogLevel::INFO, "Analysis complete.");
    log(LogLevel::INFO, "Setlist: " + setlist.project_name +
        " (" + std::to_string(setlist.songs.size()) + " songs)");
    log(LogLevel::INFO, "Median LUFS: " + std::to_string(setlist.setlist_score.median_lufs));
    log(LogLevel::INFO, "Consistency: " + setlist.setlist_score.grade);

    // Completion signal
    std::cout << "{\"status\":\"complete\",\"output_dir\":\""
              << args.output_dir << "\"}" << std::endl;

    return 0;
}
