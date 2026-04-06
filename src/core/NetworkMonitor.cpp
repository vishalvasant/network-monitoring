/**
#include "core/NetworkMonitor.hpp"
#include "utils/Logger.hpp"
#include "config/ConfigManager.hpp"

#include <pcap.h>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <cstring>

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------

NetworkMonitor::NetworkMonitor(QObject* parent)
    : QObject(parent)
    , m_running(false)
    , m_handle(nullptr)
    , m_totalPackets(0)
    , m_totalBytes(0)
{
    Logger::getInstance().log(LogLevel::DEBUG, "NetworkMonitor constructed.");
}

NetworkMonitor::~NetworkMonitor() {
    stop();   // Ensure capture is stopped and resources are released
    Logger::getInstance().log(LogLevel::DEBUG, "NetworkMonitor destroyed.");
}

// ---------------------------------------------------------------------------
// Initialisation
// ---------------------------------------------------------------------------

bool NetworkMonitor::initialize() {
    auto& config = ConfigManager::getInstance();
    m_interface  = config.getInterface();

    if (m_interface.empty()) {
        // Attempt automatic interface discovery when none is configured
        char errBuf[PCAP_ERRBUF_SIZE];
        pcap_if_t* allDevs = nullptr;

        if (pcap_findalldevs(&allDevs, errBuf) == -1 || allDevs == nullptr) {
            Logger::getInstance().log(LogLevel::ERROR,
                "No network interfaces found: " + std::string(errBuf));
            return false;
        }

        m_interface = allDevs->name;   // Pick the first available interface
        pcap_freealldevs(allDevs);
        Logger::getInstance().log(LogLevel::INFO,
            "Auto-selected interface: " + m_interface);
    }

    Logger::getInstance().log(LogLevel::INFO,
        "NetworkMonitor initialised on interface: " + m_interface);
    return true;
}

// ---------------------------------------------------------------------------
// Start / Stop
// ---------------------------------------------------------------------------

void NetworkMonitor::start() {
    if (m_running.load()) {
        Logger::getInstance().log(LogLevel::WARNING,
            "NetworkMonitor::start() called while already running.");
        return;
    }

    char errBuf[PCAP_ERRBUF_SIZE];

    // Open the interface in promiscuous mode so we capture all frames,
    // not just those addressed to this host.
    m_handle = pcap_open_live(
        m_interface.c_str(),
        BUFSIZ,   // Snapshot length — maximum bytes per packet
        1,        // Promiscuous mode ON
        100,      // Read timeout in milliseconds
        errBuf
    );

    if (m_handle == nullptr) {
        Logger::getInstance().log(LogLevel::ERROR,
            "pcap_open_live failed: " + std::string(errBuf));
        return;
    }

    m_running = true;

    // Spin up a dedicated capture thread so the GUI event loop is never blocked
    m_captureThread = std::thread(&NetworkMonitor::captureLoop, this);

    Logger::getInstance().log(LogLevel::INFO,
        "Packet capture started on: " + m_interface);
    emit monitoringStarted();
}

void NetworkMonitor::stop() {
    if (!m_running.load()) return;

    m_running = false;   // Signal the capture thread to exit its loop

    if (m_captureThread.joinable()) {
        m_captureThread.join();   // Wait for clean exit
    }

    if (m_handle) {
        pcap_close(m_handle);
        m_handle = nullptr;
    }

    Logger::getInstance().log(LogLevel::INFO, "Packet capture stopped.");
    emit monitoringStopped();
}

// ---------------------------------------------------------------------------
// Capture loop (runs on m_captureThread)
// ---------------------------------------------------------------------------

void NetworkMonitor::captureLoop() {
    struct pcap_pkthdr* header = nullptr;
    const u_char*       data   = nullptr;

    while (m_running.load()) {
        const int result = pcap_next_ex(m_handle, &header, &data);

        if (result == 1) {
            // A complete packet was captured — process it
            processPacket(header, data);
        } else if (result == 0) {
            // Timeout with no packet — loop around and try again
            continue;
        } else if (result == -1) {
            // Unrecoverable error from libpcap
            Logger::getInstance().log(LogLevel::ERROR,
                "pcap_next_ex error: " + std::string(pcap_geterr(m_handle)));
            m_running = false;
            break;
        } else if (result == -2) {
            // EOF (e.g. reading from a pcap file) — stop gracefully
            Logger::getInstance().log(LogLevel::INFO, "Capture EOF reached.");
            m_running = false;
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// Packet processing
// ---------------------------------------------------------------------------

void NetworkMonitor::processPacket(const struct pcap_pkthdr* header,
                                   const u_char* data)
{
    if (!header || !data) return;

    // Parse raw bytes into a structured Packet object
    Packet packet = Packet::parse(data, header->caplen, header->ts);

    // Update running totals (thread-safe via atomics)
    ++m_totalPackets;
    m_totalBytes += header->caplen;

    // Forward to statistics engine for aggregation
    m_statistics.addPacket(packet);

    // Persist to the data store for historical queries
    m_dataStore.store(packet);

    // Emit Qt signal — connected slots run on the GUI thread via queued connection
    emit packetCaptured(packet);

    // Periodically emit updated statistics (~every second worth of packets)
    if (m_totalPackets % 100 == 0) {
        emit statsUpdated(m_statistics.getSnapshot());
    }
}

// ---------------------------------------------------------------------------
// Accessors
// ---------------------------------------------------------------------------

bool NetworkMonitor::isRunning() const {
    return m_running.load();
}

uint64_t NetworkMonitor::getTotalPackets() const {
    return m_totalPackets.load();
}

uint64_t NetworkMonitor::getTotalBytes() const {
    return m_totalBytes.load();
}

std::string NetworkMonitor::getInterface() const {
    return m_interface;
}

Statistics NetworkMonitor::getStatistics() const {
    return m_statistics;
}