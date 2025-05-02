#include "gui/ConnectionsWidget.hpp"
#include <QHeaderView>

ConnectionsWidget::ConnectionsWidget(NetworkMonitor* monitor, QWidget* parent)
    : QWidget(parent)
    , monitor_(monitor) {
    
    setupUI();
}

void ConnectionsWidget::setupUI() {
    auto* layout = new QVBoxLayout(this);

    // Create labels for connection statistics
    total_connections_label_ = new QLabel("Total Connections: 0", this);
    active_connections_label_ = new QLabel("Active Connections: 0", this);

    layout->addWidget(total_connections_label_);
    layout->addWidget(active_connections_label_);

    // Create table for active connections
    connections_table_ = new QTableWidget(this);
    connections_table_->setColumnCount(6);
    connections_table_->setHorizontalHeaderLabels({
        "Protocol", "Source", "Destination", "Packets", "Bytes", "Duration"
    });
    connections_table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    connections_table_->setEditTriggers(QTableWidget::NoEditTriggers);
    connections_table_->setSelectionBehavior(QTableWidget::SelectRows);
    connections_table_->setSelectionMode(QTableWidget::SingleSelection);
    connections_table_->setSortingEnabled(true);

    layout->addWidget(connections_table_);
}

void ConnectionsWidget::update() {
    updateTable();
    updateLabels();
}

void ConnectionsWidget::updateTable() {
    auto connections = monitor_->getActiveConnections();
    
    connections_table_->setRowCount(connections.size());
    int row = 0;

    for (const auto& conn : connections) {
        connections_table_->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(conn.protocol)));
        connections_table_->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(conn.source)));
        connections_table_->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(conn.destination)));
        connections_table_->setItem(row, 3, new QTableWidgetItem(QString::number(conn.packet_count)));
        connections_table_->setItem(row, 4, new QTableWidgetItem(QString::number(conn.byte_count)));
        connections_table_->setItem(row, 5, new QTableWidgetItem(formatDuration(conn.duration_seconds)));
        
        row++;
    }
}

void ConnectionsWidget::updateLabels() {
    auto connections = monitor_->getActiveConnections();
    
    total_connections_label_->setText(QString("Total Connections: %1")
        .arg(monitor_->getTotalConnections()));
    
    active_connections_label_->setText(QString("Active Connections: %1")
        .arg(connections.size()));
}

QString ConnectionsWidget::formatDuration(uint64_t seconds) const {
    if (seconds < 60) {
        return QString("%1s").arg(seconds);
    } else if (seconds < 3600) {
        return QString("%1m %2s").arg(seconds / 60).arg(seconds % 60);
    } else {
        return QString("%1h %2m %3s")
            .arg(seconds / 3600)
            .arg((seconds % 3600) / 60)
            .arg(seconds % 60);
    }
} 