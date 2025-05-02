#pragma once

#include <QMainWindow>
#include <QTimer>
#include <QTabWidget>
#include <QStatusBar>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QSettings>
#include <memory>
#include "core/NetworkMonitor.hpp"
#include "gui/StatisticsWidget.hpp"
#include "gui/ConnectionsWidget.hpp"
#include "gui/PacketsWidget.hpp"
#include "gui/BandwidthWidget.hpp"
#include "gui/FilterDialog.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(NetworkMonitor* monitor, QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void updateDisplay();
    void showFilterDialog();
    void clearFilter();
    void saveStatistics();
    void exportData();
    void showSettings();
    void showAbout();
    void toggleMonitoring(bool start);
    void changeTheme(const QString& theme);

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createDockWindows();
    void loadSettings();
    void saveSettings();
    void setupConnections();

    NetworkMonitor* monitor_;
    QTimer* update_timer_;
    QTabWidget* tab_widget_;
    StatisticsWidget* statistics_widget_;
    ConnectionsWidget* connections_widget_;
    PacketsWidget* packets_widget_;
    BandwidthWidget* bandwidth_widget_;

    // Actions
    QAction* start_action_;
    QAction* stop_action_;
    QAction* filter_action_;
    QAction* clear_filter_action_;
    QAction* save_action_;
    QAction* export_action_;
    QAction* settings_action_;
    QAction* about_action_;
    QAction* exit_action_;

    // Menus
    QMenu* file_menu_;
    QMenu* view_menu_;
    QMenu* tools_menu_;
    QMenu* help_menu_;

    // Toolbars
    QToolBar* main_toolbar_;
    QToolBar* filter_toolbar_;

    // Settings
    QSettings settings_;
    bool is_monitoring_;
}; 