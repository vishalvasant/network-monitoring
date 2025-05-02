#include "gui/BandwidthWidget.hpp"
#include <QDateTime>

BandwidthWidget::BandwidthWidget(NetworkMonitor* monitor, QWidget* parent)
    : QWidget(parent)
    , monitor_(monitor) {
    
    setupUI();
}

void BandwidthWidget::setupUI() {
    auto* layout = new QVBoxLayout(this);

    // Create top bar with labels and time range selector
    auto* top_bar = new QHBoxLayout();
    
    current_bandwidth_label_ = new QLabel("Current Bandwidth: 0 bps", this);
    average_bandwidth_label_ = new QLabel("Average Bandwidth: 0 bps", this);
    
    time_range_combo_ = new QComboBox(this);
    time_range_combo_->addItems({"Last Minute", "Last 5 Minutes", "Last 15 Minutes", "Last Hour"});
    time_range_combo_->setCurrentIndex(0);
    
    top_bar->addWidget(current_bandwidth_label_);
    top_bar->addWidget(average_bandwidth_label_);
    top_bar->addStretch();
    top_bar->addWidget(new QLabel("Time Range:", this));
    top_bar->addWidget(time_range_combo_);
    
    layout->addLayout(top_bar);

    // Create chart
    chart_ = new QChart();
    chart_->setTitle("Bandwidth Usage");
    chart_->setAnimationOptions(QChart::NoAnimation);
    chart_->legend()->hide();
    
    // Create series
    bandwidth_series_ = new QLineSeries();
    chart_->addSeries(bandwidth_series_);
    
    // Create axes
    axis_x_ = new QValueAxis();
    axis_x_->setTitleText("Time (seconds)");
    axis_x_->setRange(0, 60);
    chart_->addAxis(axis_x_, Qt::AlignBottom);
    bandwidth_series_->attachAxis(axis_x_);
    
    axis_y_ = new QValueAxis();
    axis_y_->setTitleText("Bandwidth (bps)");
    axis_y_->setRange(0, 1000);
    chart_->addAxis(axis_y_, Qt::AlignLeft);
    bandwidth_series_->attachAxis(axis_y_);
    
    // Create chart view
    chart_view_ = new QChartView(chart_);
    chart_view_->setRenderHint(QPainter::Antialiasing);
    
    layout->addWidget(chart_view_);

    // Connect signals
    connect(time_range_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BandwidthWidget::updateTimeRange);
}

void BandwidthWidget::update() {
    updateChart();
    updateLabels();
}

void BandwidthWidget::updateTimeRange(int index) {
    int seconds;
    switch (index) {
        case 0: seconds = 60; break;      // Last minute
        case 1: seconds = 300; break;     // Last 5 minutes
        case 2: seconds = 900; break;     // Last 15 minutes
        case 3: seconds = 3600; break;    // Last hour
        default: seconds = 60;
    }
    
    axis_x_->setRange(0, seconds);
    updateChart();
}

void BandwidthWidget::updateChart() {
    auto bandwidth_data = monitor_->getBandwidthHistory();
    if (bandwidth_data.empty()) {
        return;
    }

    // Clear existing data
    bandwidth_series_->clear();
    
    // Get current time range
    int max_seconds = axis_x_->max();
    uint64_t current_time = QDateTime::currentSecsSinceEpoch();
    
    // Add data points
    for (const auto& [timestamp, bandwidth] : bandwidth_data) {
        int seconds_ago = current_time - timestamp;
        if (seconds_ago <= max_seconds) {
            bandwidth_series_->append(max_seconds - seconds_ago, bandwidth);
        }
    }
    
    // Update Y axis range
    double max_bandwidth = 0;
    for (const auto& point : bandwidth_series_->points()) {
        max_bandwidth = std::max(max_bandwidth, point.y());
    }
    axis_y_->setRange(0, max_bandwidth * 1.1); // Add 10% padding
}

void BandwidthWidget::updateLabels() {
    auto stats = monitor_->getStatistics();
    
    current_bandwidth_label_->setText(QString("Current Bandwidth: %1")
        .arg(formatBandwidth(stats.getCurrentBandwidth())));
    
    average_bandwidth_label_->setText(QString("Average Bandwidth: %1")
        .arg(formatBandwidth(stats.getAverageBandwidth())));
}

void BandwidthWidget::clearChart() {
    bandwidth_series_->clear();
    axis_y_->setRange(0, 1000);
}

QString BandwidthWidget::formatBandwidth(uint64_t bps) const {
    if (bps < 1000) {
        return QString("%1 bps").arg(bps);
    } else if (bps < 1000000) {
        return QString("%1 Kbps").arg(bps / 1000.0, 0, 'f', 2);
    } else if (bps < 1000000000) {
        return QString("%1 Mbps").arg(bps / 1000000.0, 0, 'f', 2);
    } else {
        return QString("%1 Gbps").arg(bps / 1000000000.0, 0, 'f', 2);
    }
} 