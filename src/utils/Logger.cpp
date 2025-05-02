#include "utils/Logger.hpp"
#include <iostream>
#include <ctime>

std::unique_ptr<std::ofstream> Logger::log_file_;
Logger::Level Logger::current_level_ = Logger::Level::INFO;
std::mutex Logger::mutex_;
bool Logger::initialized_ = false;

void Logger::init(const std::string& log_file, Level level) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) {
        return;
    }

    log_file_ = std::make_unique<std::ofstream>(log_file, std::ios::app);
    if (!log_file_->is_open()) {
        throw std::runtime_error("Failed to open log file: " + log_file);
    }

    current_level_ = level;
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