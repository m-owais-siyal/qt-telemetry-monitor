# Qt Telemetry Monitor

Real-time UDP telemetry monitor built with Qt 6 and C++17.

<!-- TODO (Sunday): 30-second GIF or video here, then 2 screenshots -->

## Features

<!-- TODO: fill in once they work -->
- Receives telemetry over UDP multicast or unicast on a worker thread
- Detects lost and late packets from sequence numbers, including wraparound
- Batches updates every 50 ms so the UI stays smooth at high packet rates
- Live charts and per-channel min / max / average
- Simulator with configurable rate and packet loss

## Architecture

```
telemetry-sim ──UDP──▶ UdpReceiver (worker thread)
                          │ batched samples + stats (queued signals)
                          ▼
                    TelemetryController ──▶ ChannelModel
                          │
                          ▼
                       QML UI (display only)
```

## Build

Requirements: Qt 6.11 or newer (Core, Network, Quick, Graphs, Test), CMake 3.21+, a C++17 compiler.
Tested with MSVC 2022 on Windows.

```
cmake -B build -DCMAKE_PREFIX_PATH=C:/Qt/6.11.0/msvc2022_64
cmake --build build --config Release
```

Or open `CMakeLists.txt` in Qt Creator.

## Run

```
telemetry-sim --rate 50 --drop 2
telemetry-monitor
```

`telemetry-sim --help` lists all options.

## Tests

```
ctest --test-dir build -C Release --output-on-failure
```

On Windows, Qt's `bin` folder must be on `PATH` when running tests from the command line.

## License

GPL-3.0
