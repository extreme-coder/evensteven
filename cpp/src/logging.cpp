#include "evensteven/logging.h"
#include <iostream>
#include <mutex>

namespace evensteven {

static LogLevel g_log_level = LogLevel::INFO;
static bool g_verbose = false;
static std::mutex g_log_mutex;

void set_log_level(LogLevel level) {
    g_log_level = level;
}

void set_verbose(bool verbose) {
    g_verbose = verbose;
    if (verbose) {
        g_log_level = LogLevel::DEBUG;
    }
}

void log(LogLevel level, const std::string& message) {
    if (level < g_log_level) return;

    std::lock_guard<std::mutex> lock(g_log_mutex);

    const char* prefix = "";
    switch (level) {
        case LogLevel::DEBUG: prefix = "[DEBUG] "; break;
        case LogLevel::INFO:  prefix = "[INFO]  "; break;
        case LogLevel::WARN:  prefix = "[WARN]  "; break;
        case LogLevel::ERROR: prefix = "[ERROR] "; break;
    }

    std::cerr << prefix << message << std::endl;
}

void report_progress(const std::string& stage, float percent, const std::string& song) {
    std::lock_guard<std::mutex> lock(g_log_mutex);
    std::cout << "{\"progress\":{\"stage\":\"" << stage
              << "\",\"percent\":" << percent;
    if (!song.empty()) {
        std::cout << ",\"song\":\"" << song << "\"";
    }
    std::cout << "}}" << std::endl;
}

} // namespace evensteven
