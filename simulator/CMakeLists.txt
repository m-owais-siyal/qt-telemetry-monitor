qt_add_executable(telemetry-sim
    main.cpp
)

target_link_libraries(telemetry-sim PRIVATE
    telemetry_common
    Qt6::Network
)
