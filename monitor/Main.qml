pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtGraphs
import TelemetryMonitor

// Layout skeleton only. Styling, a StatCard.qml file and the status pill
// colours are tonight's job.
ApplicationWindow {
    id: window

    width: 1280
    height: 800
    visible: true
    title: qsTr("Telemetry Monitor")
    color: "#0f1115"

    component StatCard: Rectangle {
        id: card

        property string label
        property string value

        Layout.fillWidth: true
        Layout.preferredHeight: 88
        radius: 12
        color: "#171a21"

        Column {
            anchors.left: parent.left
            anchors.leftMargin: 16
            anchors.verticalCenter: parent.verticalCenter
            spacing: 4

            Label { text: card.label; color: "#8b93a1"; font.pixelSize: 13 }
            Label { text: card.value; color: "#e6e8eb"; font.pixelSize: 26; font.bold: true }
        }
    }

    function statusText(status) {
        switch (status) {
        case TelemetryController.Receiving: return qsTr("Receiving")
        case TelemetryController.Stale: return qsTr("No data")
        case TelemetryController.Error: return qsTr("Error")
        default: return qsTr("Idle")
        }
    }

    function formatValue(value) {
        return value === undefined ? "—" : Number(value).toFixed(2)
    }

    TelemetryController {
        id: controller
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        // Sidebar: connection settings and start/stop
        Rectangle {
            Layout.preferredWidth: 260
            Layout.fillHeight: true
            radius: 12
            color: "#171a21"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12

                Label { text: qsTr("Connection"); color: "#e6e8eb"; font.pixelSize: 18; font.bold: true }
                Label { text: qsTr("Address"); color: "#8b93a1" }
                TextField {
                    Layout.fillWidth: true
                    text: controller.address
                    enabled: !controller.running
                    onEditingFinished: controller.address = text
                }
                Label { text: qsTr("Port"); color: "#8b93a1" }
                SpinBox {
                    Layout.fillWidth: true
                    from: 1
                    to: 65535
                    editable: true
                    value: controller.port
                    enabled: !controller.running
                    onValueModified: controller.port = value
                }
                Button {
                    Layout.fillWidth: true
                    text: controller.running ? qsTr("Stop") : qsTr("Start")
                    onClicked: controller.running ? controller.stop() : controller.start()
                }
                Label { text: window.statusText(controller.status); color: "#8b93a1" }
                Item { Layout.fillHeight: true }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            RowLayout {
                Layout.fillWidth: true
                spacing: 16

                StatCard { label: qsTr("Packets/s"); value: controller.packetsPerSecond.toFixed(1) }
                StatCard { label: qsTr("Received"); value: controller.receivedPackets }
                StatCard { label: qsTr("Lost"); value: controller.lostPackets }
                StatCard { label: qsTr("Loss"); value: controller.lossPercent.toFixed(2) + " %" }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 12
                color: "#171a21"

                GraphsView {
                    anchors.fill: parent
                    anchors.margins: 12

                    theme: GraphsTheme {
                        colorScheme: GraphsTheme.ColorScheme.Dark
                        backgroundVisible: false
                    }
                    // TODO: channels have very different ranges; consider one chart per channel.
                    axisX: ValueAxis { min: 0; max: 10 }
                    axisY: ValueAxis { min: 0; max: 120 }

                    LineSeries { id: temperatureSeries; name: qsTr("Temperature") }
                    LineSeries { id: pressureSeries; name: qsTr("Pressure") }
                    LineSeries { id: voltageSeries; name: qsTr("Voltage") }

                    Component.onCompleted: {
                        controller.bindSeries(0, temperatureSeries)
                        controller.bindSeries(1, pressureSeries)
                        controller.bindSeries(2, voltageSeries)
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 170
                radius: 12
                color: "#171a21"

                ListView {
                    anchors.fill: parent
                    anchors.margins: 12
                    interactive: false
                    model: controller.channels

                    delegate: RowLayout {
                        id: row

                        required property string name
                        required property string unit
                        required property var value
                        required property var minimum
                        required property var maximum
                        required property var average

                        width: ListView.view.width
                        height: 44

                        Label { text: row.name; color: "#e6e8eb"; Layout.preferredWidth: 160 }
                        Label { text: window.formatValue(row.value) + " " + row.unit; color: "#e6e8eb"; Layout.fillWidth: true }
                        Label { text: qsTr("min ") + window.formatValue(row.minimum); color: "#8b93a1"; Layout.fillWidth: true }
                        Label { text: qsTr("max ") + window.formatValue(row.maximum); color: "#8b93a1"; Layout.fillWidth: true }
                        Label { text: qsTr("avg ") + window.formatValue(row.average); color: "#8b93a1"; Layout.fillWidth: true }
                    }
                }
            }
        }
    }
}
