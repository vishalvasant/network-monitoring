cmake_minimum_required(VERSION 3.15)
project(NetworkMonitor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find required packages
find_package(PCAP REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(OpenSSL REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Charts)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PCAP_INCLUDE_DIRS}
    ${SQLite3_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${Protobuf_INCLUDE_DIRS}
    ${gRPC_INCLUDE_DIRS}
)

# Source files
set(SOURCES
    src/main.cpp
    src/core/NetworkMonitor.cpp
    src/core/Packet.cpp
    src/core/Statistics.cpp
    src/storage/DataStore.cpp
    src/utils/Logger.cpp
    src/config/ConfigManager.cpp
    src/gui/MainWindow.cpp
    src/gui/FilterDialog.cpp
    src/gui/StatisticsWidget.cpp
    src/gui/ConnectionsWidget.cpp
    src/gui/PacketsWidget.cpp
    src/gui/BandwidthWidget.cpp
)

# Header files
set(HEADERS
    include/core/NetworkMonitor.hpp
    include/core/Packet.hpp
    include/core/Statistics.hpp
    include/storage/DataStore.hpp
    include/utils/Logger.hpp
    include/config/ConfigManager.hpp
    include/gui/MainWindow.hpp
    include/gui/FilterDialog.hpp
    include/gui/StatisticsWidget.hpp
    include/gui/ConnectionsWidget.hpp
    include/gui/PacketsWidget.hpp
    include/gui/BandwidthWidget.hpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${PCAP_LIBRARIES}
    SQLite::SQLite3
    Boost::system
    Boost::filesystem
    OpenSSL::SSL
    OpenSSL::Crypto
    protobuf::libprotobuf
    gRPC::grpc++
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Charts
)

# Install
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Install configuration files
install(FILES config/default.conf
    DESTINATION etc/${PROJECT_NAME}
)

# Install documentation
install(FILES README.md
    DESTINATION share/doc/${PROJECT_NAME}
) 