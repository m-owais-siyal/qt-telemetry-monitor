qt_add_executable(telemetry-monitor
    main.cpp
)

qt_add_qml_module(telemetry-monitor
    URI TelemetryMonitor
    VERSION 1.0
    QML_FILES
        Main.qml
    SOURCES
        UdpReceiver.h UdpReceiver.cpp
        ChannelModel.h ChannelModel.cpp
        TelemetryController.h TelemetryController.cpp
)

target_link_libraries(telemetry-monitor PRIVATE
    telemetry_common
    Qt6::Quick
    Qt6::Network
    Qt6::Graphs
)

set_target_properties(telemetry-monitor PROPERTIES
    WIN32_EXECUTABLE ON
)
