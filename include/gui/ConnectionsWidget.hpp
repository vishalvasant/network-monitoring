#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QLabel>
#include "core/NetworkMonitor.hpp"

class ConnectionsWidget : public QWidget {
    Q_OBJECT

public:
    explicit ConnectionsWidget(NetworkMonitor* monitor, QWidget* parent = nullptr);
    void update();

private:
    NetworkMonitor* monitor_;
    QTableWidget* connections_table_;
    QLabel* total_connections_label_;
    QLabel* active_connections_label_;

    void setupUI();
    void updateTable();
    void updateLabels();
    QString formatDuration(uint64_t seconds) const;
}; 