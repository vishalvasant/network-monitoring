#include "core/NetworkMonitor.hpp"
#include <mutex>

NetworkMonitor::NetworkMonitor() {}

NetworkMonitor::~NetworkMonitor() {
    stop();
}

void NetworkMonitor::registerPacketCallback(PacketCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    packet_callbacks_.push_back(callback);
}

void NetworkMonitor::unregisterPacketCallback(PacketCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    packet_callbacks_.erase(
        std::remove_if(packet_callbacks_.begin(), packet_callbacks_.end(),
            [callback](const PacketCallback& cb) {
                return cb.target<void(const Packet&)>() == callback.target<void(const Packet&)>();
            }
        ),
        packet_callbacks_.end()
    );
}

void NetworkMonitor::processPacket(const Packet& packet) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    for (const auto& callback : packet_callbacks_) {
        callback(packet);
    }
}