import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import TelemetryMonitor

ColumnLayout {
    id: sidebar

    required property TelemetryController controller

    spacing: Theme.spacing

    Card {
        Layout.fillWidth: true
        title: qsTr("Connection")

        Label {
            text: qsTr("Address")
            color: Theme.textSecondary
            font.pixelSize: 12
        }

        AppTextField {
            id: addressField

            Layout.fillWidth: true
            text: sidebar.controller.address
            placeholderText: "239.255.42.1"
            enabled: !sidebar.controller.running
            onEditingFinished: sidebar.controller.address = text
        }

        Label {
            Layout.topMargin: 4
            text: qsTr("Port")
            color: Theme.textSecondary
            font.pixelSize: 12
        }

        AppTextField {
            id: portField

            Layout.fillWidth: true
            text: sidebar.controller.port.toString()
            validator: IntValidator { bottom: 1; top: 65535 }
            enabled: !sidebar.controller.running
            onEditingFinished: sidebar.controller.port = parseInt(text)
        }

        Label {
            Layout.fillWidth: true
            text: qsTr("A multicast group to join, or a local address such as 0.0.0.0 to receive unicast.")
            wrapMode: Text.Wrap
            color: Theme.textMuted
            font.pixelSize: 12
        }

        AppButton {
            Layout.fillWidth: true
            Layout.topMargin: 6
            text: sidebar.controller.running ? qsTr("Stop") : qsTr("Start listening")
            tone: sidebar.controller.running ? Theme.danger : Theme.accent
            filled: !sidebar.controller.running
            onClicked: {
                if (sidebar.controller.running) {
                    sidebar.controller.stop()
                    return
                }
                // Commit fields that still have focus before starting.
                sidebar.controller.address = addressField.text
                sidebar.controller.port = parseInt(portField.text)
                sidebar.controller.start()
            }
        }

        Label {
            Layout.fillWidth: true
            visible: sidebar.controller.errorMessage.length > 0
            text: sidebar.controller.errorMessage
            wrapMode: Text.Wrap
            color: Theme.danger
            font.pixelSize: 12
        }
    }

    Card {
        Layout.fillWidth: true
        title: qsTr("Time window")

        SegmentedControl {
            Layout.fillWidth: true
            options: [10, 30, 60]
            suffix: " s"
            currentValue: sidebar.controller.windowSeconds
            onActivated: value => sidebar.controller.windowSeconds = value
        }
    }

    Card {
        Layout.fillWidth: true
        Layout.fillHeight: true
        title: qsTr("Events")

        headerItems: Label {
            text: qsTr("Clear")
            color: clearHover.hovered ? Theme.textPrimary : Theme.textMuted
            font.pixelSize: 12
            visible: eventList.count > 0

            HoverHandler {
                id: clearHover
                cursorShape: Qt.PointingHandCursor
            }

            TapHandler {
                onTapped: sidebar.controller.events.clear()
            }
        }

        EventList {
            id: eventList

            Layout.fillWidth: true
            Layout.fillHeight: true
            model: sidebar.controller.events
        }
    }
}
