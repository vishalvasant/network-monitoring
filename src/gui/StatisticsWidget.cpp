#include "gui/StatisticsWidget.hpp"
#include <QHeaderView>

StatisticsWidget::StatisticsWidget(NetworkMonitor* monitor, QWidget* parent)
    : QWidget(parent)
    , monitor_(monitor) {
    
    setupUI();
}

void StatisticsWidget::setupUI() {
    auto* layout = new QVBoxLayout(this);

    // Create labels for summary statistics
    total_packets_label_ = new QLabel("Total Packets: 0", this);
    total_bytes_label_ = new QLabel("Total Bytes: 0", this);
    current_bandwidth_label_ = new QLabel("Current Bandwidth: 0 bps", this);
    average_packet_size_label_ = new QLabel("Average Packet Size: 0 bytes", this);

    layout->addWidget(total_packets_label_);
    layout->addWidget(total_bytes_label_);
    layout->addWidget(current_bandwidth_label_);
    layout->addWidget(average_packet_size_label_);

    // Create table for protocol statistics
    stats_table_ = new QTableWidget(this);
    stats_table_->setColumnCount(4);
    stats_table_->setHorizontalHeaderLabels({"Protocol", "Packets", "Bytes", "Percentage"});
    stats_table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    stats_table_->setEditTriggers(QTableWidget::NoEditTriggers);
    stats_table_->setSelectionBehavior(QTableWidget::SelectRows);
    stats_table_->setSelectionMode(QTableWidget::SingleSelection);

    layout->addWidget(stats_table_);
}

void StatisticsWidget::update() {
    updateTable();
    updateLabels();
}

void StatisticsWidget::updateTable() {
    auto stats = monitor_->getStatistics();
    auto protocol_stats = stats.getProtocolStatistics();

    stats_table_->setRowCount(protocol_stats.size());
    int row = 0;

    for (const auto& [protocol, count] : protocol_stats) {
        stats_table_->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(protocol)));
        stats_table_->setItem(row, 1, new QTableWidgetItem(QString::number(count)));
        
        // Calculate bytes and percentage
        uint64_t bytes = stats.getProtocolBytes(protocol);
        double percentage = (count * 100.0) / stats.getTotalPackets();
        
        stats_table_->setItem(row, 2, new QTableWidgetItem(QString::number(bytes)));
        stats_table_->setItem(row, 3, new QTableWidgetItem(QString::number(percentage, 'f', 2) + "%"));
        
        row++;
    }
}

void StatisticsWidget::updateLabels() {
    auto stats = monitor_->getStatistics();
    
    total_packets_label_->setText(QString("Total Packets: %1")
        .arg(stats.getTotalPackets()));
    
    total_bytes_label_->setText(QString("Total Bytes: %1")
        .arg(stats.getTotalBytes()));
    
    current_bandwidth_label_->setText(QString("Current Bandwidth: %1 bps")
        .arg(stats.getCurrentBandwidth()));
    
    if (stats.getTotalPackets() > 0) {
        double avg_size = static_cast<double>(stats.getTotalBytes()) / stats.getTotalPackets();
        average_packet_size_label_->setText(QString("Average Packet Size: %1 bytes")
            .arg(avg_size, 0, 'f', 2));
    } else {
        average_packet_size_label_->setText("Average Packet Size: 0 bytes");
    }
} 