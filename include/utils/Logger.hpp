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

    struct Config {
        size_t max_file_size = 10 * 1024 * 1024; // 10MB default
        size_t max_backup_files = 5;             // 5 backups default
        std::string log_file = "network_monitor.log";
        Level level = Level::INFO;
    };

    static void init(const Config& config = Config());
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
    static void rotateIfNeeded();
    static void rotateLogs();
    static size_t getCurrentFileSize();
    static std::string getBackupFileName(size_t index);

    static std::unique_ptr<std::ofstream> log_file_;
    static Level current_level_;
    static std::mutex mutex_;
    static bool initialized_;
    static Config config_;
}; 