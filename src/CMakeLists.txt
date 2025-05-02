# Core components
add_subdirectory(core)
add_subdirectory(protocols)
add_subdirectory(analysis)
add_subdirectory(utils)
add_subdirectory(storage)
add_subdirectory(api)
add_subdirectory(cli)
add_subdirectory(gui)
add_subdirectory(plugins)
add_subdirectory(config)

# Main executable
add_executable(network_monitor
    main.cpp
)

target_link_libraries(network_monitor
    PRIVATE
    network_monitor_core
    network_monitor_protocols
    network_monitor_analysis
    network_monitor_utils
    network_monitor_storage
    network_monitor_api
    network_monitor_cli
    network_monitor_gui
    network_monitor_plugins
    network_monitor_config
    Threads::Threads
    Boost::system
    Boost::filesystem
    OpenSSL::SSL
    OpenSSL::Crypto
    PCAP::PCAP
    protobuf::libprotobuf
    gRPC::grpc++
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
) 