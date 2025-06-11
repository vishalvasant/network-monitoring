#include "utils/Logger.hpp"
#include <iostream>
#include <ctime>
#include <filesystem>

std::unique_ptr<std::ofstream> Logger::log_file_;
Logger::Level Logger::current_level_ = Logger::Level::INFO;
std::mutex Logger::mutex_;
bool Logger::initialized_ = false;
Logger::Config Logger::config_;
std::string Logger::base_log_file_;

void Logger::init(const Config& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) {
        return;
    }
    config_ = config;
    base_log_file_ = config.log_file;
    log_file_ = std::make_unique<std::ofstream>(base_log_file_, std::ios::app);
    if (!log_file_->is_open()) {
        throw std::runtime_error("Failed to open log file: " + base_log_file_);
    }
    current_level_ = config.level;
    initialized_ = true;
    info("Logger initialized");
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

void Logger::log(Level level, const std::string& message) {
    if (!initialized_) {
        std::cerr << "Logger not initialized" << std::endl;
        return;
    }
    if (level < current_level_) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    rotateLogsIfNeeded();
    std::string timestamp = getTimestamp();
    std::string level_str = levelToString(level);
    std::string log_message = timestamp + " [" + level_str + "] " + message + "\n";
    *log_file_ << log_message;
    log_file_->flush();
    if (level >= Level::ERROR) {
        std::cerr << log_message;
    }
}

void Logger::rotateLogsIfNeeded() {
    if (getCurrentLogFileSize() < config_.max_file_size) {
        return;
    }
    log_file_->close();
    performLogRotation();
    log_file_ = std::make_unique<std::ofstream>(base_log_file_, std::ios::trunc);
}

void Logger::performLogRotation() {
    namespace fs = std::filesystem;
    // Remove the oldest backup if max_backup_files is reached
    if (config_.max_backup_files > 0) {
        std::string oldest = getBackupFileName(config_.max_backup_files);
        if (fs::exists(oldest)) {
            fs::remove(oldest);
        }
        // Shift backups
        for (size_t i = config_.max_backup_files; i > 1; --i) {
            std::string prev = getBackupFileName(i - 1);
            std::string next = getBackupFileName(i);
            if (fs::exists(prev)) {
                fs::rename(prev, next);
            }
        }
        // Rename current log to .1
        if (fs::exists(base_log_file_)) {
            fs::rename(base_log_file_, getBackupFileName(1));
        }
    }
}

size_t Logger::getCurrentLogFileSize() {
    namespace fs = std::filesystem;
    if (fs::exists(base_log_file_)) {
        return fs::file_size(base_log_file_);
    }
    return 0;
}

std::string Logger::getBackupFileName(size_t index) {
    return base_log_file_ + "." + std::to_string(index);
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