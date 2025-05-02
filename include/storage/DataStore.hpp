#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <atomic>
#include <chrono>
#include <sqlite3.h>
#include "protocols/Packet.hpp"

class DataStore {
public:
    DataStore(const std::string& db_path = "network_monitor.db");
    ~DataStore();

    void store(const Packet& packet);
    void flush();
    void close();

    // Query methods
    std::vector<Packet> getPacketsByProtocol(Packet::Protocol protocol, size_t limit = 1000);
    std::vector<Packet> getPacketsByHost(const std::string& host, size_t limit = 1000);
    std::vector<Packet> getPacketsByTimeRange(
        const std::chrono::system_clock::time_point& start,
        const std::chrono::system_clock::time_point& end,
        size_t limit = 1000
    );
    std::vector<Packet> getPacketsByConnection(
        const std::string& source_host,
        const std::string& dest_host,
        size_t limit = 1000
    );

    // Statistics queries
    uint64_t getPacketCount();
    uint64_t getByteCount();
    std::vector<std::pair<Packet::Protocol, uint64_t>> getProtocolDistribution();
    std::vector<std::pair<std::string, uint64_t>> getHostDistribution();
    std::vector<std::pair<std::string, uint64_t>> getConnectionDistribution();

private:
    void initializeDatabase();
    void createTables();
    void storeThread();
    void insertPacket(const Packet& packet);
    void batchInsert();
    std::string protocolToString(Packet::Protocol protocol) const;
    Packet::Protocol stringToProtocol(const std::string& str) const;

    sqlite3* db_;
    std::string db_path_;
    std::atomic<bool> running_;
    std::thread store_thread_;
    std::queue<Packet> packet_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    
    static constexpr size_t BATCH_SIZE = 1000;
    static constexpr std::chrono::seconds FLUSH_INTERVAL{5};
}; 