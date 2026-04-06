/**
#include "protocols/Packet.hpp"
#include "utils/Logger.hpp"

#include <netinet/ether.h>   // Ethernet header structures
#include <netinet/ip.h>      // IPv4 header structures
#include <netinet/ip6.h>     // IPv6 header structures
#include <netinet/tcp.h>     // TCP header structures
#include <netinet/udp.h>     // UDP header structures
#include <netinet/ip_icmp.h> // ICMP header structures
#include <arpa/inet.h>       // inet_ntop, ntohs, ntohl
#include <cstring>
#include <sstream>
#include <iomanip>

// ---------------------------------------------------------------------------
// Static factory
// ---------------------------------------------------------------------------

/**
Packet Packet::parse(const uint8_t* data, uint32_t length,
                     const struct timeval& ts)
{
    Packet pkt;
    pkt.m_rawLength  = length;
    pkt.m_timestamp  = ts;
    pkt.m_protocol   = Protocol::UNKNOWN;

    if (!data || length < sizeof(struct ethhdr)) {