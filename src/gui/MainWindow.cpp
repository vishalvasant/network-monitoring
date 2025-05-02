#include "gui/MainWindow.hpp"
#include "utils/Logger.hpp"
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QStyle>
#include <QStyleFactory>
#include <QScreen>

MainWindow::MainWindow(NetworkMonitor* monitor, QWidget* parent)
    : QMainWindow(parent)
    , monitor_(monitor)
    , is_monitoring_(false) {
    
    setWindowTitle("Network Monitor");
    resize(1200, 800);

    // Center window on screen
    QScreen* screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    createDockWindows();

    // Create update timer
    update_timer_ = new QTimer(this);
    connect(update_timer_, &QTimer::timeout, this, &MainWindow::updateDisplay);

    loadSettings();
    setupConnections();
}

MainWindow::~MainWindow() {
    saveSettings();
    if (is_monitoring_) {
        monitor_->stop();
    }
}

void MainWindow::createActions() {
    // Start/Stop actions
    start_action_ = new QAction(QIcon::fromTheme("media-playback-start"), "Start", this);
    start_action_->setStatusTip("Start monitoring");
    connect(start_action_, &QAction::triggered, this, [this]() { toggleMonitoring(true); });

    stop_action_ = new QAction(QIcon::fromTheme("media-playback-stop"), "Stop", this);
    stop_action_->setStatusTip("Stop monitoring");
    stop_action_->setEnabled(false);
    connect(stop_action_, &QAction::triggered, this, [this]() { toggleMonitoring(false); });

    // Filter actions
    filter_action_ = new QAction(QIcon::fromTheme("edit-find"), "Filter", this);
    filter_action_->setStatusTip("Set packet filter");
    connect(filter_action_, &QAction::triggered, this, &MainWindow::showFilterDialog);

    clear_filter_action_ = new QAction(QIcon::fromTheme("edit-clear"), "Clear Filter", this);
    clear_filter_action_->setStatusTip("Clear packet filter");
    connect(clear_filter_action_, &QAction::triggered, this, &MainWindow::clearFilter);

    // File actions
    save_action_ = new QAction(QIcon::fromTheme("document-save"), "Save Statistics", this);
    save_action_->setStatusTip("Save statistics to file");
    connect(save_action_, &QAction::triggered, this, &MainWindow::saveStatistics);

    export_action_ = new QAction(QIcon::fromTheme("document-export"), "Export Data", this);
    export_action_->setStatusTip("Export packet data to file");
    connect(export_action_, &QAction::triggered, this, &MainWindow::exportData);

    // Settings and help actions
    settings_action_ = new QAction(QIcon::fromTheme("preferences-system"), "Settings", this);
    settings_action_->setStatusTip("Configure application settings");
    connect(settings_action_, &QAction::triggered, this, &MainWindow::showSettings);

    about_action_ = new QAction(QIcon::fromTheme("help-about"), "About", this);
    about_action_->setStatusTip("About Network Monitor");
    connect(about_action_, &QAction::triggered, this, &MainWindow::showAbout);

    exit_action_ = new QAction(QIcon::fromTheme("application-exit"), "Exit", this);
    exit_action_->setStatusTip("Exit the application");
    connect(exit_action_, &QAction::triggered, this, &QWidget::close);
}

void MainWindow::createMenus() {
    // File menu
    file_menu_ = menuBar()->addMenu("&File");
    file_menu_->addAction(save_action_);
    file_menu_->addAction(export_action_);
    file_menu_->addSeparator();
    file_menu_->addAction(exit_action_);

    // View menu
    view_menu_ = menuBar()->addMenu("&View");
    view_menu_->addAction(main_toolbar_->toggleViewAction());
    view_menu_->addAction(filter_toolbar_->toggleViewAction());

    // Tools menu
    tools_menu_ = menuBar()->addMenu("&Tools");
    tools_menu_->addAction(start_action_);
    tools_menu_->addAction(stop_action_);
    tools_menu_->addSeparator();
    tools_menu_->addAction(filter_action_);
    tools_menu_->addAction(clear_filter_action_);
    tools_menu_->addSeparator();
    tools_menu_->addAction(settings_action_);

    // Help menu
    help_menu_ = menuBar()->addMenu("&Help");
    help_menu_->addAction(about_action_);
}

void MainWindow::createToolBars() {
    // Main toolbar
    main_toolbar_ = addToolBar("Main");
    main_toolbar_->addAction(start_action_);
    main_toolbar_->addAction(stop_action_);
    main_toolbar_->addSeparator();
    main_toolbar_->addAction(save_action_);
    main_toolbar_->addAction(export_action_);

    // Filter toolbar
    filter_toolbar_ = addToolBar("Filter");
    filter_toolbar_->addAction(filter_action_);
    filter_toolbar_->addAction(clear_filter_action_);
}

void MainWindow::createStatusBar() {
    statusBar()->showMessage("Ready");
}

void MainWindow::createDockWindows() {
    // Create tab widget
    tab_widget_ = new QTabWidget(this);
    setCentralWidget(tab_widget_);

    // Create widgets
    statistics_widget_ = new StatisticsWidget(monitor_, this);
    connections_widget_ = new ConnectionsWidget(monitor_, this);
    packets_widget_ = new PacketsWidget(monitor_, this);
    bandwidth_widget_ = new BandwidthWidget(monitor_, this);

    // Add widgets to tabs
    tab_widget_->addTab(statistics_widget_, "Statistics");
    tab_widget_->addTab(connections_widget_, "Connections");
    tab_widget_->addTab(packets_widget_, "Packets");
    tab_widget_->addTab(bandwidth_widget_, "Bandwidth");
}

void MainWindow::loadSettings() {
    settings_.beginGroup("MainWindow");
    restoreGeometry(settings_.value("geometry").toByteArray());
    restoreState(settings_.value("windowState").toByteArray());
    settings_.endGroup();

    // Load theme
    QString theme = settings_.value("theme", "dark").toString();
    changeTheme(theme);
}

void MainWindow::saveSettings() {
    settings_.beginGroup("MainWindow");
    settings_.setValue("geometry", saveGeometry());
    settings_.setValue("windowState", saveState());
    settings_.endGroup();
}

void MainWindow::setupConnections() {
    // Connect tab widget signals
    connect(tab_widget_, &QTabWidget::currentChanged, this, [this](int index) {
        // Update display for the current tab
        updateDisplay();
    });
}

void MainWindow::updateDisplay() {
    if (!is_monitoring_) {
        return;
    }

    // Update current tab
    switch (tab_widget_->currentIndex()) {
        case 0: // Statistics
            statistics_widget_->update();
            break;
        case 1: // Connections
            connections_widget_->update();
            break;
        case 2: // Packets
            packets_widget_->update();
            break;
        case 3: // Bandwidth
            bandwidth_widget_->update();
            break;
    }

    // Update status bar
    auto stats = monitor_->getStatistics();
    statusBar()->showMessage(QString("Packets: %1 | Bandwidth: %2 bps")
        .arg(stats.getTotalPackets())
        .arg(stats.getCurrentBandwidth()));
}

void MainWindow::showFilterDialog() {
    FilterDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString filter = dialog.getFilter();
        try {
            monitor_->setFilter(filter.toStdString());
            statusBar()->showMessage("Filter set: " + filter, 3000);
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Filter Error", e.what());
        }
    }
}

void MainWindow::clearFilter() {
    monitor_->setFilter("");
    statusBar()->showMessage("Filter cleared", 3000);
}

void MainWindow::saveStatistics() {
    QString filename = QFileDialog::getSaveFileName(this,
        "Save Statistics", "", "Text Files (*.txt);;All Files (*)");
    
    if (!filename.isEmpty()) {
        try {
            // Implementation depends on how you want to save statistics
            statusBar()->showMessage("Statistics saved to: " + filename, 3000);
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Save Error", e.what());
        }
    }
}

void MainWindow::exportData() {
    QString filename = QFileDialog::getSaveFileName(this,
        "Export Data", "", "CSV Files (*.csv);;All Files (*)");
    
    if (!filename.isEmpty()) {
        try {
            // Implementation depends on how you want to export data
            statusBar()->showMessage("Data exported to: " + filename, 3000);
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Export Error", e.what());
        }
    }
}

void MainWindow::showSettings() {
    // Implementation depends on your settings dialog
}

void MainWindow::showAbout() {
    QMessageBox::about(this, "About Network Monitor",
        "Network Monitor\n\n"
        "A comprehensive network monitoring and analysis tool.\n\n"
        "Version 1.0.0\n"
        "Copyright (C) 2024");
}

void MainWindow::toggleMonitoring(bool start) {
    if (start) {
        try {
            monitor_->start();
            is_monitoring_ = true;
            start_action_->setEnabled(false);
            stop_action_->setEnabled(true);
            update_timer_->start(1000); // Update every second
            statusBar()->showMessage("Monitoring started");
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", e.what());
        }
    } else {
        monitor_->stop();
        is_monitoring_ = false;
        start_action_->setEnabled(true);
        stop_action_->setEnabled(false);
        update_timer_->stop();
        statusBar()->showMessage("Monitoring stopped");
    }
}

void MainWindow::changeTheme(const QString& theme) {
    if (theme == "dark") {
        qApp->setStyle(QStyleFactory::create("Fusion"));
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
        darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);
        qApp->setPalette(darkPalette);
    } else {
        qApp->setStyle(QStyleFactory::create("Fusion"));
        qApp->setPalette(qApp->style()->standardPalette());
    }
} 