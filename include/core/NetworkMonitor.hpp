#pragma once

#include <string>
#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <queue>
#include <functional>
#include <pcap.h>
#include "protocols/Packet.hpp"
#include "analysis/Statistics.hpp"
#include "storage/DataStore.hpp"

class NetworkMonitor {
public:
    NetworkMonitor();
    ~NetworkMonitor();

    void start();
    void stop();
    void setInterface(const std::string& interface);
    void addPacketCallback(std::function<void(const Packet&)> callback);
    Statistics getStatistics() const;
    void setFilter(const std::string& filter);

private:
    void captureThread();
    void processThread();
    void analyzeThread();
    void storeThread();

    std::string interface_;
    pcap_t* pcap_handle_;
    std::atomic<bool> running_;
    std::thread capture_thread_;
    std::thread process_thread_;
    std::thread analyze_thread_;
    std::thread store_thread_;

    std::queue<Packet> packet_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;

    std::vector<std::function<void(const Packet&)>> packet_callbacks_;
    std::unique_ptr<Statistics> statistics_;
    std::unique_ptr<DataStore> data_store_;
    std::string filter_;
}; 