#pragma once
#include "protocols/Packet.hpp"
#include <vector>
#include <functional>
#include <mutex>

class NetworkMonitor {
public:
    using PacketCallback = std::function<void(const Packet&)>;

    NetworkMonitor();
    ~NetworkMonitor();

    void registerPacketCallback(PacketCallback callback);
    void unregisterPacketCallback(PacketCallback callback);
    void processPacket(const Packet& packet);
    void start() { /* Implementation */ }
    void stop() { /* Implementation */ }

private:
    std::vector<PacketCallback> packet_callbacks_;
    std::mutex callback_mutex_;
};