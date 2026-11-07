pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import QtQuick.Shapes
import TelemetryMonitor

ApplicationWindow {
    id: window

    width: 1440
    height: 900
    minimumWidth: 1120
    minimumHeight: 760
    visible: true
    title: qsTr("Telemetry Monitor")
    color: Theme.background

    // Set from the command line (see main.cpp).
    property bool autoStart: false
    property string initialAddress
    property int initialPort: 0

    Component.onCompleted: {
        if (initialAddress.length > 0)
            telemetry.address = initialAddress
        if (initialPort > 0)
            telemetry.port = initialPort
        if (autoStart)
            telemetry.start()
    }

    TelemetryController {
        id: telemetry
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: Theme.spacing

        // Header
        RowLayout {
            Layout.fillWidth: true
            spacing: 14

            Rectangle {
                implicitWidth: 42
                implicitHeight: 42
                radius: 11
                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0.0; color: Theme.accent }
                    GradientStop { position: 1.0; color: Theme.accentAlt }
                }

                Shape {
                    anchors.fill: parent
                    preferredRendererType: Shape.CurveRenderer

                    ShapePath {
                        strokeColor: "#ffffff"
                        strokeWidth: 2.5
                        fillColor: "transparent"
                        capStyle: ShapePath.RoundCap
                        joinStyle: ShapePath.RoundJoin

                        PathPolyline {
                            path: [Qt.point(8, 23), Qt.point(14, 23), Qt.point(18, 13), Qt.point(24, 31),
                                   Qt.point(28, 19), Qt.point(34, 19)]
                        }
                    }
                }
            }

            ColumnLayout {
                spacing: 2

                Label {
                    text: qsTr("Telemetry Monitor")
                    color: Theme.textPrimary
                    font.pixelSize: 20
                    font.weight: Font.Bold
                }

                Label {
                    text: telemetry.running ? qsTr("UDP %1:%2").arg(telemetry.address).arg(telemetry.port)
                                             : qsTr("Not listening")
                    color: Theme.textSecondary
                    font.pixelSize: 13
                }
            }

            Item { Layout.fillWidth: true }

            StatusPill {
                status: telemetry.status
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Theme.spacing

            Sidebar {
                Layout.fillWidth: false
                Layout.preferredWidth: 340
                Layout.fillHeight: true
                controller: telemetry
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: Theme.spacing

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Theme.spacing

                    StatCard {
                        Layout.fillWidth: true
                        Layout.preferredWidth: 1
                        label: qsTr("Packets / s")
                        value: telemetry.packetsPerSecond.toFixed(1)
                        caption: qsTr("1-second average")
                    }

                    StatCard {
                        Layout.fillWidth: true
                        Layout.preferredWidth: 1
                        label: qsTr("Received")
                        value: telemetry.receivedPackets.toLocaleString(Qt.locale(), "f", 0)
                        caption: qsTr("this session")
                    }

                    StatCard {
                        Layout.fillWidth: true
                        Layout.preferredWidth: 1
                        label: qsTr("Lost")
                        value: telemetry.lostPackets.toLocaleString(Qt.locale(), "f", 0)
                        valueColor: telemetry.lostPackets > 0 ? Theme.warning : Theme.textPrimary
                        caption: qsTr("from sequence gaps")
                    }

                    StatCard {
                        Layout.fillWidth: true
                        Layout.preferredWidth: 1
                        label: qsTr("Loss rate")
                        value: telemetry.lossPercent.toFixed(2) + " %"
                        valueColor: telemetry.lossPercent >= 5 ? Theme.danger
                                  : telemetry.lossPercent > 0 ? Theme.warning
                                  : telemetry.receivedPackets > 0 ? Theme.success : Theme.textPrimary
                        caption: qsTr("lost / (received + lost)")
                    }
                }

                Repeater {
                    model: telemetry.channels

                    delegate: ChannelChart {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        lineColor: Theme.channelColors[index % Theme.channelColors.length]
                        viewStart: telemetry.viewStart
                        viewEnd: telemetry.viewEnd
                        running: telemetry.running
                        Component.onCompleted: telemetry.bindSeries(index, series)
                    }
                }
            }
        }
    }
}
