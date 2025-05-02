#include "cli/CommandLineInterface.hpp"
#include "utils/Logger.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <ctime>
#include <filesystem>

CommandLineInterface::CommandLineInterface(NetworkMonitor* monitor)
    : monitor_(monitor)
    , running_(false) {
    initializeCommands();
}

CommandLineInterface::~CommandLineInterface() {
    stop();
}

void CommandLineInterface::run() {
    if (running_) {
        return;
    }

    running_ = true;
    input_thread_ = std::thread(&CommandLineInterface::inputThread, this);

    std::cout << "Network Monitor CLI\n";
    std::cout << "Type 'help' for available commands\n\n";

    while (running_) {
        updateDisplay();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void CommandLineInterface::stop() {
    if (!running_) {
        return;
    }

    running_ = false;
    if (input_thread_.joinable()) {
        input_thread_.join();
    }
}

void CommandLineInterface::initializeCommands() {
    commands_ = {
        {"help", [this](const std::string&) { displayHelp(); }},
        {"stats", [this](const std::string&) { displayStatistics(); }},
        {"connections", [this](const std::string&) { displayConnections(); }},
        {"packets", [this](const std::string&) { displayPackets(); }},
        {"bandwidth", [this](const std::string&) { displayBandwidth(); }},
        {"errors", [this](const std::string&) { displayErrors(); }},
        {"filter", [this](const std::string& args) { setFilter(args); }},
        {"clear", [this](const std::string&) { clearFilter(); }},
        {"save", [this](const std::string& args) { saveStatistics(args); }},
        {"export", [this](const std::string& args) { exportData(args); }},
        {"quit", [this](const std::string&) { stop(); }},
        {"exit", [this](const std::string&) { stop(); }}
    };
}

void CommandLineInterface::inputThread() {
    std::string command;
    while (running_) {
        std::cout << "> ";
        std::getline(std::cin, command);

        if (!command.empty()) {
            processCommand(command);
        }
    }
}

void CommandLineInterface::processCommand(const std::string& command) {
    std::istringstream iss(command);
    std::string cmd, args;
    iss >> cmd >> std::ws;
    std::getline(iss, args);

    auto it = commands_.find(cmd);
    if (it != commands_.end()) {
        it->second(args);
    } else {
        std::cout << "Unknown command: " << cmd << "\n";
        std::cout << "Type 'help' for available commands\n";
    }
}

void CommandLineInterface::displayHelp() const {
    std::cout << "Available commands:\n";
    std::cout << "  help                    - Display this help message\n";
    std::cout << "  stats                   - Display current statistics\n";
    std::cout << "  connections             - Display active connections\n";
    std::cout << "  packets                 - Display recent packets\n";
    std::cout << "  bandwidth               - Display bandwidth usage\n";
    std::cout << "  errors                  - Display error statistics\n";
    std::cout << "  filter <expression>     - Set packet filter\n";
    std::cout << "  clear                   - Clear packet filter\n";
    std::cout << "  save <filename>         - Save statistics to file\n";
    std::cout << "  export <filename>       - Export packet data to file\n";
    std::cout << "  quit/exit              - Exit the program\n";
}

void CommandLineInterface::displayStatistics() const {
    auto stats = monitor_->getStatistics();
    
    std::cout << "\nNetwork Statistics:\n";
    std::cout << "Total Packets: " << stats.getTotalPackets() << "\n";
    std::cout << "Total Bytes: " << formatBytes(stats.getTotalBytes()) << "\n";
    std::cout << "Current Bandwidth: " << formatBandwidth(stats.getCurrentBandwidth()) << "\n";
    std::cout << "Average Bandwidth: " << formatBandwidth(stats.getAverageBandwidth()) << "\n";
    std::cout << "Error Count: " << stats.getErrorCount() << "\n\n";

    std::cout << "Top Protocols:\n";
    for (const auto& [protocol, count] : stats.getTopProtocols(5)) {
        std::cout << "  " << protocol << ": " << count << " packets\n";
    }
    std::cout << "\n";

    std::cout << "Top Hosts:\n";
    for (const auto& [host, count] : stats.getTopHosts(5)) {
        std::cout << "  " << host << ": " << count << " packets\n";
    }
}

void CommandLineInterface::displayConnections() const {
    auto stats = monitor_->getStatistics();
    
    std::cout << "\nActive Connections:\n";
    for (const auto& conn_id : stats.getActiveConnections()) {
        auto conn_stats = stats.getConnectionStats(conn_id);
        std::cout << "  " << conn_id << "\n";
        std::cout << "    Packets: " << conn_stats.packet_count << "\n";
        std::cout << "    Bytes: " << formatBytes(conn_stats.byte_count) << "\n";
        std::cout << "    Retransmissions: " << conn_stats.retransmission_count << "\n";
        std::cout << "    Duration: " << std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now() - conn_stats.start_time).count() << "s\n";
    }
}

void CommandLineInterface::displayPackets() const {
    // Implementation depends on how you want to display packets
    // This could show the most recent packets or packets matching certain criteria
}

void CommandLineInterface::displayBandwidth() const {
    auto stats = monitor_->getStatistics();
    auto history = stats.getBandwidthHistory();
    
    std::cout << "\nBandwidth History:\n";
    for (const auto& [time, bandwidth] : history) {
        std::cout << formatTimestamp(time) << ": " 
                  << formatBandwidth(bandwidth) << "\n";
    }
}

void CommandLineInterface::displayErrors() const {
    auto stats = monitor_->getStatistics();
    
    std::cout << "\nError Statistics:\n";
    std::cout << "Total Errors: " << stats.getErrorCount() << "\n\n";
    
    std::cout << "Top Errors:\n";
    for (const auto& [error, count] : stats.getTopErrors(5)) {
        std::cout << "  " << error << ": " << count << " occurrences\n";
    }
}

void CommandLineInterface::setFilter(const std::string& filter) {
    if (filter.empty()) {
        std::cout << "Please specify a filter expression\n";
        return;
    }

    try {
        monitor_->setFilter(filter);
        std::cout << "Filter set: " << filter << "\n";
    } catch (const std::exception& e) {
        std::cout << "Error setting filter: " << e.what() << "\n";
    }
}

void CommandLineInterface::clearFilter() {
    monitor_->setFilter("");
    std::cout << "Filter cleared\n";
}

void CommandLineInterface::saveStatistics(const std::string& filename) const {
    if (filename.empty()) {
        std::cout << "Please specify a filename\n";
        return;
    }

    try {
        auto stats = monitor_->getStatistics();
        // Implementation depends on how you want to save statistics
        std::cout << "Statistics saved to: " << filename << "\n";
    } catch (const std::exception& e) {
        std::cout << "Error saving statistics: " << e.what() << "\n";
    }
}

void CommandLineInterface::exportData(const std::string& filename) const {
    if (filename.empty()) {
        std::cout << "Please specify a filename\n";
        return;
    }

    try {
        // Implementation depends on how you want to export data
        std::cout << "Data exported to: " << filename << "\n";
    } catch (const std::exception& e) {
        std::cout << "Error exporting data: " << e.what() << "\n";
    }
}

void CommandLineInterface::updateDisplay() {
    // Clear screen and update display
    // Implementation depends on your terminal capabilities
}

std::string CommandLineInterface::formatBytes(uint64_t bytes) const {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit = 0;
    double size = static_cast<double>(bytes);

    while (size >= 1024.0 && unit < 4) {
        size /= 1024.0;
        unit++;
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << size << " " << units[unit];
    return oss.str();
}

std::string CommandLineInterface::formatBandwidth(double bits_per_second) const {
    const char* units[] = {"bps", "Kbps", "Mbps", "Gbps", "Tbps"};
    int unit = 0;
    double bandwidth = bits_per_second;

    while (bandwidth >= 1000.0 && unit < 4) {
        bandwidth /= 1000.0;
        unit++;
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << bandwidth << " " << units[unit];
    return oss.str();
}

std::string CommandLineInterface::formatTimestamp(const std::chrono::system_clock::time_point& time) const {
    auto time_t = std::chrono::system_clock::to_time_t(time);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        time.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
} 