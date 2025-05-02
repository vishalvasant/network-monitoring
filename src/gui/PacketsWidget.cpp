#include "gui/PacketsWidget.hpp"
#include <QHeaderView>
#include <QMessageBox>
#include <QDateTime>
#include <QDialog>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QDialogButtonBox>

PacketsWidget::PacketsWidget(NetworkMonitor* monitor, QWidget* parent)
    : QWidget(parent)
    , monitor_(monitor) {
    
    setupUI();
}

void PacketsWidget::setupUI() {
    auto* layout = new QVBoxLayout(this);

    // Create top bar with label and clear button
    auto* top_bar = new QHBoxLayout();
    total_packets_label_ = new QLabel("Total Packets: 0", this);
    clear_button_ = new QPushButton("Clear", this);
    clear_button_->setIcon(QIcon::fromTheme("edit-clear"));
    
    top_bar->addWidget(total_packets_label_);
    top_bar->addStretch();
    top_bar->addWidget(clear_button_);
    
    layout->addLayout(top_bar);

    // Create table for packets
    packets_table_ = new QTableWidget(this);
    packets_table_->setColumnCount(7);
    packets_table_->setHorizontalHeaderLabels({
        "Time", "Protocol", "Source", "Destination", "Length", "Info", "Flags"
    });
    packets_table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    packets_table_->setEditTriggers(QTableWidget::NoEditTriggers);
    packets_table_->setSelectionBehavior(QTableWidget::SelectRows);
    packets_table_->setSelectionMode(QTableWidget::SingleSelection);
    packets_table_->setSortingEnabled(true);

    layout->addWidget(packets_table_);

    // Connect signals
    connect(clear_button_, &QPushButton::clicked, this, &PacketsWidget::clearPackets);
    connect(packets_table_, &QTableWidget::doubleClicked, this, &PacketsWidget::showPacketDetails);
}

void PacketsWidget::update() {
    updateTable();
    updateLabels();
}

void PacketsWidget::updateTable() {
    auto packets = monitor_->getRecentPackets();
    
    packets_table_->setRowCount(packets.size());
    int row = 0;

    for (const auto& packet : packets) {
        packets_table_->setItem(row, 0, new QTableWidgetItem(formatTimestamp(packet.timestamp)));
        packets_table_->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(packet.protocol)));
        packets_table_->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(packet.source)));
        packets_table_->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(packet.destination)));
        packets_table_->setItem(row, 4, new QTableWidgetItem(formatPacketSize(packet.length)));
        packets_table_->setItem(row, 5, new QTableWidgetItem(QString::fromStdString(packet.info)));
        packets_table_->setItem(row, 6, new QTableWidgetItem(QString::fromStdString(packet.flags)));
        
        row++;
    }
}

void PacketsWidget::updateLabels() {
    total_packets_label_->setText(QString("Total Packets: %1")
        .arg(monitor_->getStatistics().getTotalPackets()));
}

void PacketsWidget::clearPackets() {
    monitor_->clearRecentPackets();
    update();
}

void PacketsWidget::showPacketDetails(const QModelIndex& index) {
    auto packets = monitor_->getRecentPackets();
    if (index.row() >= 0 && index.row() < packets.size()) {
        const auto& packet = packets[index.row()];
        
        QDialog dialog(this);
        dialog.setWindowTitle("Packet Details");
        dialog.resize(600, 400);

        auto* layout = new QVBoxLayout(&dialog);
        
        auto* text_edit = new QTextEdit(&dialog);
        text_edit->setReadOnly(true);
        
        // Format packet details
        QString details;
        details += QString("Time: %1\n").arg(formatTimestamp(packet.timestamp));
        details += QString("Protocol: %1\n").arg(QString::fromStdString(packet.protocol));
        details += QString("Source: %1\n").arg(QString::fromStdString(packet.source));
        details += QString("Destination: %1\n").arg(QString::fromStdString(packet.destination));
        details += QString("Length: %1\n").arg(formatPacketSize(packet.length));
        details += QString("Info: %1\n").arg(QString::fromStdString(packet.info));
        details += QString("Flags: %1\n").arg(QString::fromStdString(packet.flags));
        
        // Add hex dump if available
        if (!packet.hex_dump.empty()) {
            details += "\nHex Dump:\n";
            details += QString::fromStdString(packet.hex_dump);
        }
        
        text_edit->setText(details);
        layout->addWidget(text_edit);
        
        auto* button_box = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
        connect(button_box, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        layout->addWidget(button_box);
        
        dialog.exec();
    }
}

QString PacketsWidget::formatTimestamp(uint64_t timestamp) const {
    QDateTime date_time;
    date_time.setSecsSinceEpoch(timestamp);
    return date_time.toString("yyyy-MM-dd hh:mm:ss.zzz");
}

QString PacketsWidget::formatPacketSize(uint32_t size) const {
    if (size < 1024) {
        return QString("%1 B").arg(size);
    } else if (size < 1024 * 1024) {
        return QString("%1 KB").arg(size / 1024.0, 0, 'f', 2);
    } else {
        return QString("%1 MB").arg(size / (1024.0 * 1024.0), 0, 'f', 2);
    }
} 