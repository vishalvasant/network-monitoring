#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "core/NetworkMonitor.hpp"

class PacketsWidget : public QWidget {
    Q_OBJECT

public:
    explicit PacketsWidget(NetworkMonitor* monitor, QWidget* parent = nullptr);
    void update();

private slots:
    void clearPackets();
    void showPacketDetails(const QModelIndex& index);

private:
    NetworkMonitor* monitor_;
    QTableWidget* packets_table_;
    QLabel* total_packets_label_;
    QPushButton* clear_button_;

    void setupUI();
    void updateTable();
    void updateLabels();
    QString formatTimestamp(uint64_t timestamp) const;
    QString formatPacketSize(uint32_t size) const;
}; 