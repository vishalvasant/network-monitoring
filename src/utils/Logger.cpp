#include "utils/Logger.hpp"
#include <iostream>
#include <ctime>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

Logger::Config Logger::config_;
std::unique_ptr<std::ofstream> Logger::log_file_;
Logger::Level Logger::current_level_ = Logger::Level::INFO;
std::mutex Logger::mutex_;
bool Logger::initialized_ = false;

void Logger::init(const Config& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) {
        return;
    }

    config_ = config;
    current_level_ = config.level;

    // Ensure the directory exists
    fs::path log_path(config_.log_file);
    if (log_path.has_parent_path() && !fs::exists(log_path.parent_path())) {
        fs::create_directories(log_path.parent_path());
    }

    // Open the log file
    log_file_ = std::make_unique<std::ofstream>(config_.log_file, std::ios::app);
    if (!log_file_->is_open()) {
        throw std::runtime_error("Failed to open log file: " + config_.log_file);
    }

    initialized_ = true;
    info("Logger initialized with log rotation (max size: " + 
         std::to_string(config_.max_file_size / (1024 * 1024)) + "MB, " +
         "max backups: " + std::to_string(config_.max_backup_files) + ")");
}

void Logger::init(const std::string& log_file, Level level) {
    Config config;
    config.log_file = log_file;
    config.level = level;
    init(config);
}

void Logger::setLevel(Level level) {
    std::lock_guard<std::mutex> lock(mutex_);
    current_level_ = level;
}

void Logger::debug(const std::string& message) {
    log(Level::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(Level::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(Level::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(Level::ERROR, message);
}

void Logger::fatal(const std::string& message) {
    log(Level::FATAL, message);
}

std::string Logger::getBackupFilename(int index) const {
    if (index == 0) {
        return config_.log_file;
    }
    return config_.log_file + "." + std::to_string(index);
}

void Logger::rotateLogIfNeeded() {
    if (!log_file_) {
        return;
    }

    // Get current file size
    log_file_->flush();
    std::error_code ec;
    auto file_size = fs::file_size(config_.log_file, ec);
    
    // If there was an error or file size is within limits, do nothing
    if (ec || file_size < config_.max_file_size) {
        return;
    }

    // Close the current log file
    log_file_.reset();
    
    // Perform the rotation
    rotateLog();
    
    // Reopen the log file
    log_file_ = std::make_unique<std::ofstream>(config_.log_file, std::ios::app);
    if (!log_file_->is_open()) {
        throw std::runtime_error("Failed to reopen log file after rotation: " + config_.log_file);
    }
}

void Logger::rotateLog() {
    // Remove the oldest backup if it exists
    std::string oldest_backup = getBackupFilename(config_.max_backup_files);
    if (fs::exists(oldest_backup)) {
        fs::remove(oldest_backup);
    }

    // Shift all backup files
    for (int i = config_.max_backup_files - 1; i >= 0; --i) {
        std::string source = getBackupFilename(i);
        std::string dest = getBackupFilename(i + 1);
        
        if (fs::exists(source)) {
            fs::rename(source, dest);
        }
    }
}

void Logger::log(Level level, const std::string& message) {
    if (!initialized_) {
        std::cerr << "Logger not initialized" << std::endl;
        return;
    }

    if (level < current_level_) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if we need to rotate the log
    rotateLogIfNeeded();
    
    std::string timestamp = getTimestamp();
    std::string level_str = levelToString(level);
    
    std::string log_message = timestamp + " [" + level_str + "] " + message + "\n";
    
    *log_file_ << log_message;
    log_file_->flush();

    // Also output to console for ERROR and FATAL levels
    if (level >= Level::ERROR) {
        std::cerr << log_message;
    }
}

std::string Logger::levelToString(Level level) {
    switch (level) {
        case Level::DEBUG:   return "DEBUG";
        case Level::INFO:    return "INFO";
        case Level::WARNING: return "WARNING";
        case Level::ERROR:   return "ERROR";
        case Level::FATAL:   return "FATAL";
        default:            return "UNKNOWN";
    }
}

std::string Logger::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
       << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
} 