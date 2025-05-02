# Network Monitor

A comprehensive network monitoring and analysis tool written in C++. This project provides real-time network traffic monitoring, protocol analysis, and statistical reporting capabilities.

## Features

- Real-time packet capture and analysis
- Support for multiple network protocols (TCP, UDP, ICMP, HTTP, HTTPS, DNS, DHCP, ARP)
- Detailed packet inspection and protocol parsing
- Network statistics and bandwidth monitoring
- Connection tracking and analysis
- SQLite database for packet storage and historical analysis
- Both CLI and GUI interfaces
- Extensible plugin system
- Comprehensive logging system

## Requirements

- C++20 compatible compiler
- CMake 3.15 or higher
- libpcap
- SQLite3
- Boost (system, filesystem)
- OpenSSL
- Protocol Buffers
- gRPC
- Qt6 (for GUI)

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

### Command Line Interface

```bash
./network_monitor --interface eth0 --cli
```

Options:
- `--interface, -i`: Network interface to monitor
- `--config, -c`: Configuration file path
- `--cli`: Launch CLI interface
- `--gui`: Launch GUI interface
- `--log-level, -l`: Log level (debug, info, warning, error)
- `--help, -h`: Show help message

### GUI Interface

```bash
./network_monitor --interface eth0 --gui
```

## Project Structure

```
network_monitor/
├── src/
│   ├── core/           # Core monitoring functionality
│   ├── protocols/      # Protocol parsers and analyzers
│   ├── analysis/       # Statistical analysis
│   ├── utils/          # Utility functions
│   ├── storage/        # Database and storage
│   ├── api/           # API interfaces
│   ├── cli/           # Command line interface
│   ├── gui/           # Graphical user interface
│   ├── plugins/       # Plugin system
│   └── config/        # Configuration management
├── include/           # Header files
├── tests/            # Unit and integration tests
├── docs/             # Documentation
├── examples/         # Example configurations
└── scripts/          # Build and utility scripts
```

## Configuration

The application can be configured through a configuration file. Example configuration:

```ini
[general]
log_level = info
log_file = network_monitor.log
database = network_monitor.db

[monitoring]
interface = eth0
promiscuous_mode = true
buffer_size = 65536
timeout = 1000

[storage]
max_packets = 1000000
cleanup_interval = 3600

[analysis]
bandwidth_window = 60
connection_timeout = 300
```

## Contributing

1. Fork the repository
2. Create your feature branch
3. Commit your changes
4. Push to the branch
5. Create a new Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- libpcap for packet capture
- SQLite for data storage
- Boost for utilities
- Qt for GUI framework 