#include "storage/DataStore.hpp"
#include "utils/Logger.hpp"
#include <sstream>
#include <iomanip>
#include <ctime>

DataStore::DataStore(const std::string& db_path)
    : db_(nullptr)
    , db_path_(db_path)
    , running_(false) {
    initializeDatabase();
    running_ = true;
    store_thread_ = std::thread(&DataStore::storeThread, this);
}

DataStore::~DataStore() {
    close();
}

void DataStore::initializeDatabase() {
    int rc = sqlite3_open(db_path_.c_str(), &db_);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to open database: " + std::string(sqlite3_errmsg(db_)));
    }
    createTables();
}

void DataStore::createTables() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS packets (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp INTEGER NOT NULL,
            protocol TEXT NOT NULL,
            source_address TEXT NOT NULL,
            destination_address TEXT NOT NULL,
            source_port INTEGER,
            destination_port INTEGER,
            length INTEGER NOT NULL,
            is_fragmented BOOLEAN NOT NULL,
            is_malformed BOOLEAN NOT NULL,
            sequence_number INTEGER,
            acknowledgment_number INTEGER,
            window_size INTEGER,
            ttl INTEGER,
            tos INTEGER,
            payload BLOB
        );

        CREATE INDEX IF NOT EXISTS idx_packets_timestamp ON packets(timestamp);
        CREATE INDEX IF NOT EXISTS idx_packets_protocol ON packets(protocol);
        CREATE INDEX IF NOT EXISTS idx_packets_source ON packets(source_address);
        CREATE INDEX IF NOT EXISTS idx_packets_destination ON packets(destination_address);
    )";

    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::string error = err_msg;
        sqlite3_free(err_msg);
        throw std::runtime_error("Failed to create tables: " + error);
    }
}

void DataStore::store(const Packet& packet) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    packet_queue_.push(packet);
    queue_cv_.notify_one();
}

void DataStore::flush() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    batchInsert();
}

void DataStore::close() {
    if (running_) {
        running_ = false;
        queue_cv_.notify_all();
        if (store_thread_.joinable()) {
            store_thread_.join();
        }
        flush();
        if (db_) {
            sqlite3_close(db_);
            db_ = nullptr;
        }
    }
}

void DataStore::storeThread() {
    while (running_) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        queue_cv_.wait_for(lock, FLUSH_INTERVAL, [this] {
            return !running_ || packet_queue_.size() >= BATCH_SIZE;
        });

        if (!running_ && packet_queue_.empty()) {
            break;
        }

        batchInsert();
    }
}

void DataStore::insertPacket(const Packet& packet) {
    const char* sql = R"(
        INSERT INTO packets (
            timestamp, protocol, source_address, destination_address,
            source_port, destination_port, length, is_fragmented,
            is_malformed, sequence_number, acknowledgment_number,
            window_size, ttl, tos, payload
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
    }

    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        packet.timestamp.time_since_epoch()).count();

    sqlite3_bind_int64(stmt, 1, timestamp);
    sqlite3_bind_text(stmt, 2, protocolToString(packet.protocol).c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, packet.source_address.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, packet.destination_address.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, packet.source_port);
    sqlite3_bind_int(stmt, 6, packet.destination_port);
    sqlite3_bind_int64(stmt, 7, packet.length);
    sqlite3_bind_int(stmt, 8, packet.is_fragmented);
    sqlite3_bind_int(stmt, 9, packet.is_malformed);
    sqlite3_bind_int64(stmt, 10, packet.sequence_number);
    sqlite3_bind_int64(stmt, 11, packet.acknowledgment_number);
    sqlite3_bind_int(stmt, 12, packet.window_size);
    sqlite3_bind_int(stmt, 13, packet.ttl);
    sqlite3_bind_int(stmt, 14, packet.tos);
    
    if (!packet.payload.empty()) {
        sqlite3_bind_blob(stmt, 15, packet.payload.data(), packet.payload.size(), SQLITE_STATIC);
    } else {
        sqlite3_bind_null(stmt, 15);
    }

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        throw std::runtime_error("Failed to insert packet: " + std::string(sqlite3_errmsg(db_)));
    }
}

void DataStore::batchInsert() {
    if (packet_queue_.empty()) {
        return;
    }

    sqlite3_exec(db_, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);

    try {
        while (!packet_queue_.empty()) {
            insertPacket(packet_queue_.front());
            packet_queue_.pop();
        }
        sqlite3_exec(db_, "COMMIT", nullptr, nullptr, nullptr);
    } catch (const std::exception& e) {
        sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
        throw;
    }
}

std::string DataStore::protocolToString(Packet::Protocol protocol) const {
    switch (protocol) {
        case Packet::Protocol::UNKNOWN: return "UNKNOWN";
        case Packet::Protocol::ETHERNET: return "ETHERNET";
        case Packet::Protocol::IPV4: return "IPv4";
        case Packet::Protocol::IPV6: return "IPv6";
        case Packet::Protocol::TCP: return "TCP";
        case Packet::Protocol::UDP: return "UDP";
        case Packet::Protocol::ICMP: return "ICMP";
        case Packet::Protocol::HTTP: return "HTTP";
        case Packet::Protocol::HTTPS: return "HTTPS";
        case Packet::Protocol::DNS: return "DNS";
        case Packet::Protocol::DHCP: return "DHCP";
        case Packet::Protocol::ARP: return "ARP";
        default: return "UNKNOWN";
    }
}

Packet::Protocol DataStore::stringToProtocol(const std::string& str) const {
    if (str == "ETHERNET") return Packet::Protocol::ETHERNET;
    if (str == "IPv4") return Packet::Protocol::IPV4;
    if (str == "IPv6") return Packet::Protocol::IPV6;
    if (str == "TCP") return Packet::Protocol::TCP;
    if (str == "UDP") return Packet::Protocol::UDP;
    if (str == "ICMP") return Packet::Protocol::ICMP;
    if (str == "HTTP") return Packet::Protocol::HTTP;
    if (str == "HTTPS") return Packet::Protocol::HTTPS;
    if (str == "DNS") return Packet::Protocol::DNS;
    if (str == "DHCP") return Packet::Protocol::DHCP;
    if (str == "ARP") return Packet::Protocol::ARP;
    return Packet::Protocol::UNKNOWN;
}

std::vector<Packet> DataStore::getPacketsByProtocol(Packet::Protocol protocol, size_t limit) {
    const char* sql = R"(
        SELECT * FROM packets
        WHERE protocol = ?
        ORDER BY timestamp DESC
        LIMIT ?
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
    }

    sqlite3_bind_text(stmt, 1, protocolToString(protocol).c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, limit);

    std::vector<Packet> packets;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        // Convert row to Packet object
        // Implementation depends on your Packet class constructor
    }

    sqlite3_finalize(stmt);
    return packets;
}

// Similar implementations for other query methods...

uint64_t DataStore::getPacketCount() {
    const char* sql = "SELECT COUNT(*) FROM packets";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
    }

    uint64_t count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int64(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

uint64_t DataStore::getByteCount() {
    const char* sql = "SELECT SUM(length) FROM packets";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
    }

    uint64_t count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int64(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

std::vector<std::pair<Packet::Protocol, uint64_t>> DataStore::getProtocolDistribution() {
    const char* sql = R"(
        SELECT protocol, COUNT(*) as count
        FROM packets
        GROUP BY protocol
        ORDER BY count DESC
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
    }

    std::vector<std::pair<Packet::Protocol, uint64_t>> distribution;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* protocol_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        uint64_t count = sqlite3_column_int64(stmt, 1);
        distribution.emplace_back(stringToProtocol(protocol_str), count);
    }

    sqlite3_finalize(stmt);
    return distribution;
}

// Similar implementations for getHostDistribution and getConnectionDistribution... 