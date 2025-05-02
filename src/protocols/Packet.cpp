#include "protocols/Packet.hpp"
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/icmp6.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <cstring>

Packet::Packet(const uint8_t* data, size_t length, const struct timeval& timestamp)
    : raw_data(data, data + length)
    , length(length)
    , timestamp(std::chrono::system_clock::from_time_t(timestamp.tv_sec) +
                std::chrono::microseconds(timestamp.tv_usec))
    , protocol(Protocol::UNKNOWN)
    , source_port(0)
    , destination_port(0)
    , is_fragmented(false)
    , is_malformed(false)
    , sequence_number(0)
    , acknowledgment_number(0)
    , window_size(0)
    , ttl(0)
    , tos(0)
    , payload_offset(0)
    , payload_length(0) {
    
    parseEthernet();
}

void Packet::parseEthernet() {
    if (length < sizeof(struct ether_header)) {
        is_malformed = true;
        return;
    }

    const struct ether_header* eth_header = reinterpret_cast<const struct ether_header*>(raw_data.data());
    uint16_t ether_type = ntohs(eth_header->ether_type);

    switch (ether_type) {
        case ETHERTYPE_IP:
            protocol = Protocol::IPV4;
            parseIPv4();
            break;
        case ETHERTYPE_IPV6:
            protocol = Protocol::IPV6;
            parseIPv6();
            break;
        case ETHERTYPE_ARP:
            protocol = Protocol::ARP;
            parseARP();
            break;
        default:
            protocol = Protocol::UNKNOWN;
            break;
    }
}

void Packet::parseIPv4() {
    if (length < sizeof(struct ether_header) + sizeof(struct ip)) {
        is_malformed = true;
        return;
    }

    const struct ip* ip_header = reinterpret_cast<const struct ip*>(raw_data.data() + sizeof(struct ether_header));
    
    char src_addr[INET_ADDRSTRLEN];
    char dst_addr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(ip_header->ip_src), src_addr, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &(ip_header->ip_dst), dst_addr, INET_ADDRSTRLEN);
    
    source_address = src_addr;
    destination_address = dst_addr;
    ttl = ip_header->ip_ttl;
    tos = ip_header->ip_tos;

    is_fragmented = (ntohs(ip_header->ip_off) & IP_OFFMASK) != 0;

    switch (ip_header->ip_p) {
        case IPPROTO_TCP:
            protocol = Protocol::TCP;
            parseTCP();
            break;
        case IPPROTO_UDP:
            protocol = Protocol::UDP;
            parseUDP();
            break;
        case IPPROTO_ICMP:
            protocol = Protocol::ICMP;
            parseICMP();
            break;
    }
}

void Packet::parseIPv6() {
    if (length < sizeof(struct ether_header) + sizeof(struct ip6_hdr)) {
        is_malformed = true;
        return;
    }

    const struct ip6_hdr* ip6_header = reinterpret_cast<const struct ip6_hdr*>(raw_data.data() + sizeof(struct ether_header));
    
    char src_addr[INET6_ADDRSTRLEN];
    char dst_addr[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &(ip6_header->ip6_src), src_addr, INET6_ADDRSTRLEN);
    inet_ntop(AF_INET6, &(ip6_header->ip6_dst), dst_addr, INET6_ADDRSTRLEN);
    
    source_address = src_addr;
    destination_address = dst_addr;
    ttl = ip6_header->ip6_hlim;

    switch (ip6_header->ip6_nxt) {
        case IPPROTO_TCP:
            protocol = Protocol::TCP;
            parseTCP();
            break;
        case IPPROTO_UDP:
            protocol = Protocol::UDP;
            parseUDP();
            break;
        case IPPROTO_ICMPV6:
            protocol = Protocol::ICMP;
            parseICMP();
            break;
    }
}

void Packet::parseTCP() {
    if (length < sizeof(struct ether_header) + sizeof(struct ip) + sizeof(struct tcphdr)) {
        is_malformed = true;
        return;
    }

    const struct tcphdr* tcp_header = reinterpret_cast<const struct tcphdr*>(
        raw_data.data() + sizeof(struct ether_header) + sizeof(struct ip));
    
    source_port = ntohs(tcp_header->th_sport);
    destination_port = ntohs(tcp_header->th_dport);
    sequence_number = ntohl(tcp_header->th_seq);
    acknowledgment_number = ntohl(tcp_header->th_ack);
    window_size = ntohs(tcp_header->th_win);

    payload_offset = sizeof(struct ether_header) + sizeof(struct ip) + (tcp_header->th_off * 4);
    payload_length = length - payload_offset;
    
    if (payload_length > 0) {
        payload.assign(raw_data.begin() + payload_offset, raw_data.begin() + payload_offset + payload_length);
    }

    determineApplicationProtocol();
}

void Packet::parseUDP() {
    if (length < sizeof(struct ether_header) + sizeof(struct ip) + sizeof(struct udphdr)) {
        is_malformed = true;
        return;
    }

    const struct udphdr* udp_header = reinterpret_cast<const struct udphdr*>(
        raw_data.data() + sizeof(struct ether_header) + sizeof(struct ip));
    
    source_port = ntohs(udp_header->uh_sport);
    destination_port = ntohs(udp_header->uh_dport);

    payload_offset = sizeof(struct ether_header) + sizeof(struct ip) + sizeof(struct udphdr);
    payload_length = length - payload_offset;
    
    if (payload_length > 0) {
        payload.assign(raw_data.begin() + payload_offset, raw_data.begin() + payload_offset + payload_length);
    }

    determineApplicationProtocol();
}

void Packet::parseICMP() {
    // ICMP parsing implementation
}

void Packet::parseARP() {
    if (length < sizeof(struct ether_header) + sizeof(struct arphdr)) {
        is_malformed = true;
        return;
    }

    const struct arphdr* arp_header = reinterpret_cast<const struct arphdr*>(
        raw_data.data() + sizeof(struct ether_header));
    
    // ARP specific parsing
}

void Packet::determineApplicationProtocol() {
    if (isTCP()) {
        if (source_port == 80 || destination_port == 80) {
            protocol = Protocol::HTTP;
        } else if (source_port == 443 || destination_port == 443) {
            protocol = Protocol::HTTPS;
        }
    } else if (isUDP()) {
        if (source_port == 53 || destination_port == 53) {
            protocol = Protocol::DNS;
        } else if (source_port == 67 || source_port == 68 || 
                   destination_port == 67 || destination_port == 68) {
            protocol = Protocol::DHCP;
        }
    }
}

std::string Packet::getProtocolString() const {
    switch (protocol) {
        case Protocol::UNKNOWN: return "UNKNOWN";
        case Protocol::ETHERNET: return "ETHERNET";
        case Protocol::IPV4: return "IPv4";
        case Protocol::IPV6: return "IPv6";
        case Protocol::TCP: return "TCP";
        case Protocol::UDP: return "UDP";
        case Protocol::ICMP: return "ICMP";
        case Protocol::HTTP: return "HTTP";
        case Protocol::HTTPS: return "HTTPS";
        case Protocol::DNS: return "DNS";
        case Protocol::DHCP: return "DHCP";
        case Protocol::ARP: return "ARP";
        default: return "UNKNOWN";
    }
}

bool Packet::isTCP() const {
    return protocol == Protocol::TCP || protocol == Protocol::HTTP || protocol == Protocol::HTTPS;
}

bool Packet::isUDP() const {
    return protocol == Protocol::UDP || protocol == Protocol::DNS || protocol == Protocol::DHCP;
}

bool Packet::isICMP() const {
    return protocol == Protocol::ICMP;
}

bool Packet::isHTTP() const {
    return protocol == Protocol::HTTP;
}

bool Packet::isHTTPS() const {
    return protocol == Protocol::HTTPS;
}

bool Packet::isDNS() const {
    return protocol == Protocol::DNS;
}

bool Packet::isARP() const {
    return protocol == Protocol::ARP;
}

bool Packet::isIPv4() const {
    return protocol == Protocol::IPV4;
}

bool Packet::isIPv6() const {
    return protocol == Protocol::IPV6;
} 