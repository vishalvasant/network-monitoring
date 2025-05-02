#include "core/NetworkMonitor.hpp"
#include "utils/Logger.hpp"
#include <pcap.h>
#include <cstring>

NetworkMonitor::NetworkMonitor()
    : pcap_handle_(nullptr)
    , running_(false)
    , statistics_(std::make_unique<Statistics>())
    , data_store_(std::make_unique<DataStore>()) {
}

NetworkMonitor::~NetworkMonitor() {
    stop();
    if (pcap_handle_) {
        pcap_close(pcap_handle_);
    }
}

void NetworkMonitor::start() {
    if (running_) {
        return;
    }

    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_handle_ = pcap_open_live(interface_.c_str(), BUFSIZ, 1, 1000, errbuf);
    if (!pcap_handle_) {
        throw std::runtime_error("Failed to open interface: " + std::string(errbuf));
    }

    if (!filter_.empty()) {
        struct bpf_program fp;
        if (pcap_compile(pcap_handle_, &fp, filter_.c_str(), 0, PCAP_NETMASK_UNKNOWN) == -1) {
            throw std::runtime_error("Failed to compile filter: " + std::string(pcap_geterr(pcap_handle_)));
        }
        if (pcap_setfilter(pcap_handle_, &fp) == -1) {
            pcap_freecode(&fp);
            throw std::runtime_error("Failed to set filter: " + std::string(pcap_geterr(pcap_handle_)));
        }
        pcap_freecode(&fp);
    }

    running_ = true;
    capture_thread_ = std::thread(&NetworkMonitor::captureThread, this);
    process_thread_ = std::thread(&NetworkMonitor::processThread, this);
    analyze_thread_ = std::thread(&NetworkMonitor::analyzeThread, this);
    store_thread_ = std::thread(&NetworkMonitor::storeThread, this);
}

void NetworkMonitor::stop() {
    if (!running_) {
        return;
    }

    running_ = false;
    queue_cv_.notify_all();

    if (capture_thread_.joinable()) {
        capture_thread_.join();
    }
    if (process_thread_.joinable()) {
        process_thread_.join();
    }
    if (analyze_thread_.joinable()) {
        analyze_thread_.join();
    }
    if (store_thread_.joinable()) {
        store_thread_.join();
    }
}

void NetworkMonitor::setInterface(const std::string& interface) {
    interface_ = interface;
}

void NetworkMonitor::addPacketCallback(std::function<void(const Packet&)> callback) {
    packet_callbacks_.push_back(std::move(callback));
}

Statistics NetworkMonitor::getStatistics() const {
    return *statistics_;
}

void NetworkMonitor::setFilter(const std::string& filter) {
    filter_ = filter;
}

void NetworkMonitor::captureThread() {
    while (running_) {
        struct pcap_pkthdr* header;
        const u_char* data;
        int result = pcap_next_ex(pcap_handle_, &header, &data);
        
        if (result == 1) {
            Packet packet(data, header->len, header->ts);
            {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                packet_queue_.push(std::move(packet));
            }
            queue_cv_.notify_one();
        } else if (result == -1) {
            Logger::error("Error reading packet: " + std::string(pcap_geterr(pcap_handle_)));
        }
    }
}

void NetworkMonitor::processThread() {
    while (running_) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        queue_cv_.wait(lock, [this] { return !running_ || !packet_queue_.empty(); });

        if (!running_ && packet_queue_.empty()) {
            break;
        }

        Packet packet = std::move(packet_queue_.front());
        packet_queue_.pop();
        lock.unlock();

        for (const auto& callback : packet_callbacks_) {
            callback(packet);
        }
    }
}

void NetworkMonitor::analyzeThread() {
    while (running_) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        queue_cv_.wait(lock, [this] { return !running_ || !packet_queue_.empty(); });

        if (!running_ && packet_queue_.empty()) {
            break;
        }

        Packet packet = packet_queue_.front();
        lock.unlock();

        statistics_->update(packet);
    }
}

void NetworkMonitor::storeThread() {
    while (running_) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        queue_cv_.wait(lock, [this] { return !running_ || !packet_queue_.empty(); });

        if (!running_ && packet_queue_.empty()) {
            break;
        }

        Packet packet = packet_queue_.front();
        lock.unlock();

        data_store_->store(packet);
    }
} 