#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <memory>
#include <chrono>
#include <sstream>
#include <iomanip>

class Logger {
public:
    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        FATAL
    };

    static void init(const std::string& log_file = "network_monitor.log", Level level = Level::INFO);
    static void setLevel(Level level);
    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warning(const std::string& message);
    static void error(const std::string& message);
    static void fatal(const std::string& message);

private:
    static void log(Level level, const std::string& message);
    static std::string levelToString(Level level);
    static std::string getTimestamp();

    static std::unique_ptr<std::ofstream> log_file_;
    static Level current_level_;
    static std::mutex mutex_;
    static bool initialized_;
}; 