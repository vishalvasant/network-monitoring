/**
#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <iostream>
#include <memory>
#include <csignal>

#include "core/NetworkMonitor.hpp"
#include "config/ConfigManager.hpp"
#include "utils/Logger.hpp"
#include "gui/MainWindow.hpp"
#include "cli/CommandLineInterface.hpp"

/// Global pointer to the monitor — used by signal handler for clean shutdown
static NetworkMonitor* g_monitor = nullptr;

/**
void signalHandler(int signal) {
    if (g_monitor) {
        g_monitor->stop();
    }
    std::cout << "\nShutting down Network Monitor (signal " << signal << ")..." << std::endl;
    QCoreApplication::quit();
}

int main(int argc, char *argv[]) {
    // Register signal handlers for clean termination
    std::signal(SIGINT,  signalHandler);
    std::signal(SIGTERM, signalHandler);

    QApplication app(argc, argv);
    app.setApplicationName("NetworkMonitor");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Radiant");

    // -------------------------------------------------------------------------
    // Command-line argument parsing
    // -------------------------------------------------------------------------
    QCommandLineParser parser;
    parser.setApplicationDescription("Real-time network packet capture and analysis tool.");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption cliOption(
        QStringList() << "c" << "cli",
        "Run in command-line (headless) mode instead of GUI."
    );
    parser.addOption(cliOption);

    QCommandLineOption configOption(
        QStringList() << "config",
        "Path to configuration file.",
        "file"
    );
    parser.addOption(configOption);

    QCommandLineOption interfaceOption(
        QStringList() << "i" << "interface",
        "Network interface to capture packets on (e.g. eth0, en0).",
        "interface"
    );
    parser.addOption(interfaceOption);

    QCommandLineOption logLevelOption(
        QStringList() << "l" << "log-level",
        "Logging verbosity: debug, info, warning, error.",
        "level",
        "info"
    );
    parser.addOption(logLevelOption);

    parser.process(app);

    // -------------------------------------------------------------------------
    // Logger initialisation
    // -------------------------------------------------------------------------
    Logger& logger = Logger::getInstance();
    logger.setLogLevel(parser.value(logLevelOption).toStdString());
    logger.log(LogLevel::INFO, "Network Monitor starting up.");

    // -------------------------------------------------------------------------
    // Configuration loading
    // -------------------------------------------------------------------------
    ConfigManager& config = ConfigManager::getInstance();

    if (parser.isSet(configOption)) {
        const std::string cfgPath = parser.value(configOption).toStdString();
        if (!config.loadConfig(cfgPath)) {
            logger.log(LogLevel::ERROR, "Failed to load config: " + cfgPath);
            return EXIT_FAILURE;
        }
        logger.log(LogLevel::INFO, "Loaded configuration from: " + cfgPath);
    } else {
        // Fall back to the default configuration file
        config.loadConfig("config/default.conf");
    }

    // Allow the --interface flag to override whatever is in the config file
    if (parser.isSet(interfaceOption)) {
        config.setInterface(parser.value(interfaceOption).toStdString());
    }

    // -------------------------------------------------------------------------
    // Core monitor initialisation
    // -------------------------------------------------------------------------
    auto monitor = std::make_unique<NetworkMonitor>();
    g_monitor    = monitor.get();

    if (!monitor->initialize()) {
        logger.log(LogLevel::ERROR, "NetworkMonitor failed to initialise.");
        return EXIT_FAILURE;
    }

    // -------------------------------------------------------------------------
    // Launch GUI or CLI
    // -------------------------------------------------------------------------
    if (parser.isSet(cliOption)) {
        // Headless / CLI mode — useful for servers and scripted environments
        logger.log(LogLevel::INFO, "Starting in CLI mode.");
        CommandLineInterface cli(monitor.get());
        monitor->start();
        cli.run();
    } else {
        // GUI mode — default when running with a display
        logger.log(LogLevel::INFO, "Starting in GUI mode.");
        MainWindow window(monitor.get());
        window.show();
        monitor->start();
    }

    const int exitCode = app.exec();

    // -------------------------------------------------------------------------
    // Teardown
    // -------------------------------------------------------------------------
    monitor->stop();
    logger.log(LogLevel::INFO, "Network Monitor shut down cleanly. Exit code: " +
               std::to_string(exitCode));

    g_monitor = nullptr;
    return exitCode;
}