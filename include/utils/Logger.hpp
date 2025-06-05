#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <memory>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <vector>
#include <algorithm>

class Logger {
public:
    struct Config {
        std::string log_file = "network_monitor.log";
        Level level = Level::INFO;
        size_t max_file_size = 10 * 1024 * 1024; // 10MB default
        int max_backup_files = 5; // Keep 5 backup files by default
    };

    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        FATAL
    };

    static void init(const std::string& log_file = "network_monitor.log", Level level = Level::INFO);
    static void init(const Config& config);
    static Config& getConfig();
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
    static Config config_;

    // Log rotation methods
    static void rotateLogIfNeeded();
    static void rotateLog();
    static std::string getBackupFilename(int index);
    static void removeOldBackups();
};