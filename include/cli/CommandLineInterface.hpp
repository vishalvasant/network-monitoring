#pragma once

#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <functional>
#include <unordered_map>
#include "core/NetworkMonitor.hpp"

class CommandLineInterface {
public:
    explicit CommandLineInterface(NetworkMonitor* monitor);
    ~CommandLineInterface();

    void run();
    void stop();

private:
    void processCommand(const std::string& command);
    void displayHelp() const;
    void displayStatistics() const;
    void displayConnections() const;
    void displayPackets() const;
    void displayBandwidth() const;
    void displayErrors() const;
    void setFilter(const std::string& filter);
    void clearFilter();
    void saveStatistics(const std::string& filename) const;
    void exportData(const std::string& filename) const;

    NetworkMonitor* monitor_;
    std::atomic<bool> running_;
    std::thread input_thread_;
    std::unordered_map<std::string, std::function<void(const std::string&)>> commands_;

    void initializeCommands();
    void inputThread();
    void updateDisplay();
    std::string formatBytes(uint64_t bytes) const;
    std::string formatBandwidth(double bits_per_second) const;
    std::string formatTimestamp(const std::chrono::system_clock::time_point& time) const;
}; 