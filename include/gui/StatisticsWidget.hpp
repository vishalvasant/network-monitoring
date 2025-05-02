#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QLabel>
#include "core/NetworkMonitor.hpp"

class StatisticsWidget : public QWidget {
    Q_OBJECT

public:
    explicit StatisticsWidget(NetworkMonitor* monitor, QWidget* parent = nullptr);
    void update();

private:
    NetworkMonitor* monitor_;
    QTableWidget* stats_table_;
    QLabel* total_packets_label_;
    QLabel* total_bytes_label_;
    QLabel* current_bandwidth_label_;
    QLabel* average_packet_size_label_;

    void setupUI();
    void updateTable();
    void updateLabels();
}; 