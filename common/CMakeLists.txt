qt_add_library(telemetry_common STATIC
    TelemetryPacket.h TelemetryPacket.cpp
    SequenceTracker.h SequenceTracker.cpp
)

target_include_directories(telemetry_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(telemetry_common PUBLIC Qt6::Core)
