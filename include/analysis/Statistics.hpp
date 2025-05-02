#pragma once

#include <unordered_map>
#include <string>
#include <chrono>
#include <mutex>
#include <atomic>
#include <vector>
#include "protocols/Packet.hpp"

struct ProtocolStats {
    std::atomic<uint64_t> packet_count{0};
    std::atomic<uint64_t> byte_count{0};
    std::atomic<uint64_t> error_count{0};
    std::chrono::system_clock::time_point first_seen;
    std::chrono::system_clock::time_point last_seen;
};

struct HostStats {
    std::atomic<uint64_t> packet_count{0};
    std::atomic<uint64_t> byte_count{0};
    std::unordered_map<Packet::Protocol, ProtocolStats> protocol_stats;
    std::chrono::system_clock::time_point first_seen;
    std::chrono::system_clock::time_point last_seen;
};

struct ConnectionStats {
    std::atomic<uint64_t> packet_count{0};
    std::atomic<uint64_t> byte_count{0};
    std::atomic<uint64_t> retransmission_count{0};
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point last_seen;
    bool is_active;
};

class Statistics {
public:
    Statistics();
    ~Statistics() = default;

    void update(const Packet& packet);
    void reset();

    // Protocol statistics
    uint64_t getTotalPackets() const;
    uint64_t getTotalBytes() const;
    uint64_t getProtocolPacketCount(Packet::Protocol protocol) const;
    uint64_t getProtocolByteCount(Packet::Protocol protocol) const;
    std::vector<std::pair<Packet::Protocol, uint64_t>> getTopProtocols(size_t count) const;

    // Host statistics
    std::vector<std::pair<std::string, uint64_t>> getTopHosts(size_t count) const;
    HostStats getHostStats(const std::string& host) const;
    std::vector<std::string> getActiveHosts() const;

    // Connection statistics
    std::vector<std::pair<std::string, uint64_t>> getTopConnections(size_t count) const;
    ConnectionStats getConnectionStats(const std::string& connection_id) const;
    std::vector<std::string> getActiveConnections() const;

    // Bandwidth statistics
    double getCurrentBandwidth() const;
    double getAverageBandwidth() const;
    std::vector<std::pair<std::chrono::system_clock::time_point, double>> getBandwidthHistory() const;

    // Error statistics
    uint64_t getErrorCount() const;
    std::vector<std::pair<std::string, uint64_t>> getTopErrors(size_t count) const;

private:
    std::string generateConnectionId(const Packet& packet) const;
    void updateProtocolStats(const Packet& packet);
    void updateHostStats(const Packet& packet);
    void updateConnectionStats(const Packet& packet);
    void updateBandwidthStats(const Packet& packet);
    void updateErrorStats(const Packet& packet);
    void cleanupInactiveConnections();

    mutable std::mutex mutex_;
    std::atomic<uint64_t> total_packets_{0};
    std::atomic<uint64_t> total_bytes_{0};
    std::atomic<uint64_t> total_errors_{0};

    std::unordered_map<Packet::Protocol, ProtocolStats> protocol_stats_;
    std::unordered_map<std::string, HostStats> host_stats_;
    std::unordered_map<std::string, ConnectionStats> connection_stats_;

    std::vector<std::pair<std::chrono::system_clock::time_point, double>> bandwidth_history_;
    std::chrono::system_clock::time_point last_bandwidth_update_;
    std::atomic<double> current_bandwidth_{0.0};
    std::atomic<double> average_bandwidth_{0.0};

    static constexpr size_t MAX_BANDWIDTH_HISTORY = 3600; // 1 hour at 1-second intervals
    static constexpr std::chrono::seconds CONNECTION_TIMEOUT{300}; // 5 minutes
}; 