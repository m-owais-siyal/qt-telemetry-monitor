# The C++ types and the QML live in one static QML module, so the tests link
# exactly the code the application runs. The executable is just main.cpp.
qt_add_library(telemetry_monitor_lib STATIC)

set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(telemetry_monitor_lib
    URI TelemetryMonitor
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/Theme.qml
        qml/Card.qml
        qml/StatCard.qml
        qml/StatusPill.qml
        qml/ChannelChart.qml
        qml/EventList.qml
        qml/SegmentedControl.qml
        qml/AppButton.qml
        qml/AppTextField.qml
        qml/Sidebar.qml
    SOURCES
        src/TelemetryTypes.h
        src/UdpReceiver.h src/UdpReceiver.cpp
        src/ChannelModel.h src/ChannelModel.cpp
        src/EventLogModel.h src/EventLogModel.cpp
        src/TelemetryController.h src/TelemetryController.cpp
)

target_include_directories(telemetry_monitor_lib PUBLIC src)

target_link_libraries(telemetry_monitor_lib PUBLIC
    telemetry_common
    Qt6::Quick
    Qt6::Network
    Qt6::Graphs
)

qt_add_executable(telemetry-monitor
    main.cpp
)

target_link_libraries(telemetry-monitor PRIVATE
    telemetry_monitor_libplugin
    Qt6::Quick
)

set_target_properties(telemetry-monitor PROPERTIES
    WIN32_EXECUTABLE ON
)
