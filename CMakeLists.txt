cmake_minimum_required(VERSION 3.21)

project(FalconsDeck VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.11 REQUIRED COMPONENTS Core Quick Bluetooth)

qt_standard_project_setup(REQUIRES 6.11)

set(PROJECT_SOURCES
    src/main.cpp
    src/ble/BleDeviceScanner.h
    src/ble/BleDeviceScanner.cpp
    src/ble/BleRobotConnection.h
    src/ble/BleRobotConnection.cpp
    src/ble/BleConnectionManager.h
    src/ble/BleConnectionManager.cpp
    src/ble/JbdBmsConnection.h
    src/ble/JbdBmsConnection.cpp
    src/ble/FalconsRobotConnection.h
    src/ble/FalconsRobotConnection.cpp
    src/models/Robot.h
    src/models/Robot.cpp
    src/models/RobotListModel.h
    src/models/RobotListModel.cpp
    src/protocol/PacketInterface.h
    src/protocol/PacketInterface.cpp
)

qt_add_executable(FalconsDeckApp
    ${PROJECT_SOURCES}
)

qt_add_qml_module(FalconsDeckApp
    URI FalconsDeck
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/ScannerView.qml
        qml/RobotDashboard.qml
        qml/RobotCard.qml
)

target_link_libraries(FalconsDeckApp PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Bluetooth
)

set_target_properties(FalconsDeckApp PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.falcons.falconsdeck
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS FalconsDeckApp
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
