qt_add_executable(telemetry-sim
    main.cpp
    TelemetrySimulator.h TelemetrySimulator.cpp
)

target_link_libraries(telemetry-sim PRIVATE
    telemetry_common
    Qt6::Network
)
