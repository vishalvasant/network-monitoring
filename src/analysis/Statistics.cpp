#include "analysis/Statistics.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

Statistics::Statistics()
    : last_bandwidth_update_(std::chrono::system_clock::now()) {
}

void Statistics::update(const Packet& packet) {
    std::lock_guard<std::mutex> lock(mutex_);

    total_packets_++;
    total_bytes_ += packet.length;

    updateProtocolStats(packet);
    updateHostStats(packet);
    updateConnectionStats(packet);
    updateBandwidthStats(packet);
    updateErrorStats(packet);

    cleanupInactiveConnections();
}

void Statistics::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    total_packets_ = 0;
    total_bytes_ = 0;
    total_errors_ = 0;
    current_bandwidth_ = 0.0;
    average_bandwidth_ = 0.0;
    
    protocol_stats_.clear();
    host_stats_.clear();
    connection_stats_.clear();
    bandwidth_history_.clear();
    
    last_bandwidth_update_ = std::chrono::system_clock::now();
}

void Statistics::updateProtocolStats(const Packet& packet) {
    auto& stats = protocol_stats_[packet.protocol];
    stats.packet_count++;
    stats.byte_count += packet.length;
    
    if (packet.is_malformed) {
        stats.error_count++;
    }
    
    if (stats.packet_count == 1) {
        stats.first_seen = packet.timestamp;
    }
    stats.last_seen = packet.timestamp;
}

void Statistics::updateHostStats(const Packet& packet) {
    auto updateHost = [this, &packet](const std::string& host) {
        auto& stats = host_stats_[host];
        stats.packet_count++;
        stats.byte_count += packet.length;
        
        if (stats.packet_count == 1) {
            stats.first_seen = packet.timestamp;
        }
        stats.last_seen = packet.timestamp;
        
        auto& protocol_stats = stats.protocol_stats[packet.protocol];
        protocol_stats.packet_count++;
        protocol_stats.byte_count += packet.length;
        
        if (protocol_stats.packet_count == 1) {
            protocol_stats.first_seen = packet.timestamp;
        }
        protocol_stats.last_seen = packet.timestamp;
    };
    
    updateHost(packet.source_address);
    updateHost(packet.destination_address);
}

void Statistics::updateConnectionStats(const Packet& packet) {
    if (!packet.isTCP() && !packet.isUDP()) {
        return;
    }
    
    std::string connection_id = generateConnectionId(packet);
    auto& stats = connection_stats_[connection_id];
    
    stats.packet_count++;
    stats.byte_count += packet.length;
    
    if (stats.packet_count == 1) {
        stats.start_time = packet.timestamp;
        stats.is_active = true;
    }
    stats.last_seen = packet.timestamp;
    
    // Detect retransmissions for TCP
    if (packet.isTCP()) {
        // Simple retransmission detection based on sequence numbers
        // This is a basic implementation and might need improvement
        static std::unordered_map<std::string, uint32_t> last_seq;
        if (last_seq.count(connection_id) && packet.sequence_number == last_seq[connection_id]) {
            stats.retransmission_count++;
        }
        last_seq[connection_id] = packet.sequence_number;
    }
}

void Statistics::updateBandwidthStats(const Packet& packet) {
    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_bandwidth_update_).count();
    
    if (elapsed >= 1) {
        // Update bandwidth history
        bandwidth_history_.push_back({now, current_bandwidth_});
        
        // Keep only the last hour of history
        while (bandwidth_history_.size() > MAX_BANDWIDTH_HISTORY) {
            bandwidth_history_.erase(bandwidth_history_.begin());
        }
        
        // Calculate average bandwidth
        double sum = 0.0;
        for (const auto& entry : bandwidth_history_) {
            sum += entry.second;
        }
        average_bandwidth_ = sum / bandwidth_history_.size();
        
        // Reset current bandwidth
        current_bandwidth_ = 0.0;
        last_bandwidth_update_ = now;
    }
    
    current_bandwidth_ += packet.length * 8.0; // Convert bytes to bits
}

void Statistics::updateErrorStats(const Packet& packet) {
    if (packet.is_malformed) {
        total_errors_++;
    }
}

void Statistics::cleanupInactiveConnections() {
    auto now = std::chrono::system_clock::now();
    for (auto it = connection_stats_.begin(); it != connection_stats_.end();) {
        if (now - it->second.last_seen > CONNECTION_TIMEOUT) {
            it->second.is_active = false;
            it = connection_stats_.erase(it);
        } else {
            ++it;
        }
    }
}

std::string Statistics::generateConnectionId(const Packet& packet) const {
    std::stringstream ss;
    if (packet.source_address < packet.destination_address) {
        ss << packet.source_address << ":" << packet.source_port << "-"
           << packet.destination_address << ":" << packet.destination_port;
    } else {
        ss << packet.destination_address << ":" << packet.destination_port << "-"
           << packet.source_address << ":" << packet.source_port;
    }
    return ss.str();
}

uint64_t Statistics::getTotalPackets() const {
    return total_packets_;
}

uint64_t Statistics::getTotalBytes() const {
    return total_bytes_;
}

uint64_t Statistics::getProtocolPacketCount(Packet::Protocol protocol) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = protocol_stats_.find(protocol);
    return it != protocol_stats_.end() ? it->second.packet_count : 0;
}

uint64_t Statistics::getProtocolByteCount(Packet::Protocol protocol) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = protocol_stats_.find(protocol);
    return it != protocol_stats_.end() ? it->second.byte_count : 0;
}

std::vector<std::pair<Packet::Protocol, uint64_t>> Statistics::getTopProtocols(size_t count) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::pair<Packet::Protocol, uint64_t>> result;
    result.reserve(protocol_stats_.size());
    
    for (const auto& [protocol, stats] : protocol_stats_) {
        result.emplace_back(protocol, stats.packet_count);
    }
    
    std::sort(result.begin(), result.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    if (result.size() > count) {
        result.resize(count);
    }
    
    return result;
}

std::vector<std::pair<std::string, uint64_t>> Statistics::getTopHosts(size_t count) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::pair<std::string, uint64_t>> result;
    result.reserve(host_stats_.size());
    
    for (const auto& [host, stats] : host_stats_) {
        result.emplace_back(host, stats.packet_count);
    }
    
    std::sort(result.begin(), result.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    if (result.size() > count) {
        result.resize(count);
    }
    
    return result;
}

HostStats Statistics::getHostStats(const std::string& host) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = host_stats_.find(host);
    return it != host_stats_.end() ? it->second : HostStats{};
}

std::vector<std::string> Statistics::getActiveHosts() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> result;
    result.reserve(host_stats_.size());
    
    for (const auto& [host, stats] : host_stats_) {
        result.push_back(host);
    }
    
    return result;
}

std::vector<std::pair<std::string, uint64_t>> Statistics::getTopConnections(size_t count) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::pair<std::string, uint64_t>> result;
    result.reserve(connection_stats_.size());
    
    for (const auto& [conn_id, stats] : connection_stats_) {
        result.emplace_back(conn_id, stats.packet_count);
    }
    
    std::sort(result.begin(), result.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    if (result.size() > count) {
        result.resize(count);
    }
    
    return result;
}

ConnectionStats Statistics::getConnectionStats(const std::string& connection_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = connection_stats_.find(connection_id);
    return it != connection_stats_.end() ? it->second : ConnectionStats{};
}

std::vector<std::string> Statistics::getActiveConnections() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> result;
    result.reserve(connection_stats_.size());
    
    for (const auto& [conn_id, stats] : connection_stats_) {
        if (stats.is_active) {
            result.push_back(conn_id);
        }
    }
    
    return result;
}

double Statistics::getCurrentBandwidth() const {
    return current_bandwidth_;
}

double Statistics::getAverageBandwidth() const {
    return average_bandwidth_;
}

std::vector<std::pair<std::chrono::system_clock::time_point, double>> Statistics::getBandwidthHistory() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return bandwidth_history_;
}

uint64_t Statistics::getErrorCount() const {
    return total_errors_;
}

std::vector<std::pair<std::string, uint64_t>> Statistics::getTopErrors(size_t count) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::pair<std::string, uint64_t>> result;
    
    for (const auto& [protocol, stats] : protocol_stats_) {
        if (stats.error_count > 0) {
            result.emplace_back(Packet::getProtocolString(protocol), stats.error_count);
        }
    }
    
    std::sort(result.begin(), result.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    if (result.size() > count) {
        result.resize(count);
    }
    
    return result;
} 