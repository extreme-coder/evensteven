#pragma once

#include <string>

namespace evensteven {

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

void set_log_level(LogLevel level);
void set_verbose(bool verbose);
void log(LogLevel level, const std::string& message);
void report_progress(const std::string& stage, float percent, const std::string& song = "");

} // namespace evensteven
