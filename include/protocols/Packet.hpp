#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <chrono>
#include <memory>

struct Packet {
    enum class Protocol {
        UNKNOWN,
        ETHERNET,
        IPV4,
        IPV6,
        TCP,
        UDP,
        ICMP,
        HTTP,
        HTTPS,
        DNS,
        DHCP,
        ARP
    };

    Packet(const uint8_t* data, size_t length, const struct timeval& timestamp);
    ~Packet() = default;

    // Packet data
    std::vector<uint8_t> raw_data;
    size_t length;
    std::chrono::system_clock::time_point timestamp;

    // Protocol information
    Protocol protocol;
    std::string source_address;
    std::string destination_address;
    uint16_t source_port;
    uint16_t destination_port;

    // Packet analysis
    bool is_fragmented;
    bool is_malformed;
    uint32_t sequence_number;
    uint32_t acknowledgment_number;
    uint16_t window_size;
    uint8_t ttl;
    uint8_t tos;

    // Payload information
    std::vector<uint8_t> payload;
    size_t payload_offset;
    size_t payload_length;

    // Helper methods
    std::string getProtocolString() const;
    bool isTCP() const;
    bool isUDP() const;
    bool isICMP() const;
    bool isHTTP() const;
    bool isHTTPS() const;
    bool isDNS() const;
    bool isARP() const;
    bool isIPv4() const;
    bool isIPv6() const;

private:
    void parseEthernet();
    void parseIPv4();
    void parseIPv6();
    void parseTCP();
    void parseUDP();
    void parseICMP();
    void parseARP();
    void determineApplicationProtocol();
}; 