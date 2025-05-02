#include "gui/FilterDialog.hpp"

FilterDialog::FilterDialog(QWidget* parent)
    : QDialog(parent) {
    
    setWindowTitle("Set Packet Filter");
    resize(400, 150);

    // Create layout
    auto* layout = new QVBoxLayout(this);

    // Add description label
    auto* label = new QLabel("Enter BPF filter expression:", this);
    layout->addWidget(label);

    // Create filter input
    filter_edit_ = new QLineEdit(this);
    filter_edit_->setPlaceholderText("e.g., tcp port 80 or host 192.168.1.1");
    layout->addWidget(filter_edit_);

    // Create button layout
    auto* button_layout = new QHBoxLayout();
    layout->addLayout(button_layout);

    // Create buttons
    ok_button_ = new QPushButton("OK", this);
    cancel_button_ = new QPushButton("Cancel", this);

    button_layout->addWidget(ok_button_);
    button_layout->addWidget(cancel_button_);

    // Connect signals
    connect(ok_button_, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancel_button_, &QPushButton::clicked, this, &QDialog::reject);
} 