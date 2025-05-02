#include <QApplication>
#include <QCommandLineParser>
#include <QMessageBox>
#include "gui/MainWindow.hpp"
#include "core/NetworkMonitor.hpp"
#include "utils/Logger.hpp"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Network Monitor");
    app.setApplicationVersion("1.0.0");

    // Parse command line arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("Network monitoring and analysis tool");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption interfaceOption(QStringList() << "i" << "interface",
        "Network interface to monitor", "interface");
    parser.addOption(interfaceOption);

    QCommandLineOption filterOption(QStringList() << "f" << "filter",
        "BPF filter expression", "filter");
    parser.addOption(filterOption);

    QCommandLineOption logFileOption(QStringList() << "l" << "log-file",
        "Log file path", "file");
    parser.addOption(logFileOption);

    QCommandLineOption logLevelOption(QStringList() << "log-level",
        "Log level (debug, info, warning, error, fatal)", "level", "info");
    parser.addOption(logLevelOption);

    parser.process(app);

    try {
        // Initialize logger
        std::string log_file = parser.value(logFileOption).toStdString();
        if (log_file.empty()) {
            log_file = "network_monitor.log";
        }

        std::string log_level = parser.value(logLevelOption).toStdString();
        Logger::Level level = Logger::Level::INFO;
        if (log_level == "debug") level = Logger::Level::DEBUG;
        else if (log_level == "warning") level = Logger::Level::WARNING;
        else if (log_level == "error") level = Logger::Level::ERROR;
        else if (log_level == "fatal") level = Logger::Level::FATAL;

        Logger::init(log_file, level);
        Logger::info("Application started");

        // Create network monitor
        auto monitor = std::make_unique<NetworkMonitor>();

        // Set interface if specified
        std::string interface = parser.value(interfaceOption).toStdString();
        if (!interface.empty()) {
            monitor->setInterface(interface);
        }

        // Set filter if specified
        std::string filter = parser.value(filterOption).toStdString();
        if (!filter.empty()) {
            monitor->setFilter(filter);
        }

        // Create and show main window
        MainWindow main_window(monitor.get());
        main_window.show();

        // Start event loop
        int result = app.exec();

        // Cleanup
        Logger::info("Application shutting down");
        return result;

    } catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "Error",
            QString("Failed to initialize application: %1").arg(e.what()));
        return 1;
    }
} 