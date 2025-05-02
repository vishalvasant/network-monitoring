#pragma once

#include <QWidget>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include "core/NetworkMonitor.hpp"

QT_CHARTS_USE_NAMESPACE

class BandwidthWidget : public QWidget {
    Q_OBJECT

public:
    explicit BandwidthWidget(NetworkMonitor* monitor, QWidget* parent = nullptr);
    void update();

private slots:
    void updateTimeRange(int index);
    void updateChart();

private:
    NetworkMonitor* monitor_;
    QChart* chart_;
    QChartView* chart_view_;
    QLineSeries* bandwidth_series_;
    QValueAxis* axis_x_;
    QValueAxis* axis_y_;
    QLabel* current_bandwidth_label_;
    QLabel* average_bandwidth_label_;
    QComboBox* time_range_combo_;

    void setupUI();
    void updateLabels();
    void clearChart();
    QString formatBandwidth(uint64_t bps) const;
}; 