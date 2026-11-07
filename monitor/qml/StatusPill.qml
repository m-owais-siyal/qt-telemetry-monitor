import QtQuick
import QtQuick.Controls.Basic
import TelemetryMonitor

Rectangle {
    id: pill

    property int status: TelemetryController.Idle

    readonly property color tone: {
        switch (status) {
        case TelemetryController.Waiting: return Theme.accent
        case TelemetryController.Receiving: return Theme.success
        case TelemetryController.Stale: return Theme.warning
        case TelemetryController.Error: return Theme.danger
        default: return Theme.textSecondary
        }
    }

    readonly property string label: {
        switch (status) {
        case TelemetryController.Waiting: return qsTr("Waiting for data")
        case TelemetryController.Receiving: return qsTr("Receiving")
        case TelemetryController.Stale: return qsTr("Signal lost")
        case TelemetryController.Error: return qsTr("Error")
        default: return qsTr("Idle")
        }
    }

    readonly property bool pulsing: status === TelemetryController.Waiting
                                    || status === TelemetryController.Receiving

    implicitWidth: content.implicitWidth + 28
    implicitHeight: 32
    radius: height / 2
    color: Theme.withAlpha(tone, 0.12)
    border.color: Theme.withAlpha(tone, 0.35)
    border.width: 1

    Behavior on color { ColorAnimation { duration: 250 } }

    Row {
        id: content

        anchors.centerIn: parent
        spacing: 8

        Rectangle {
            anchors.verticalCenter: parent.verticalCenter
            width: 8
            height: 8
            radius: 4
            color: pill.tone

            SequentialAnimation on opacity {
                running: pill.pulsing
                loops: Animation.Infinite
                alwaysRunToEnd: true

                NumberAnimation { to: 0.25; duration: 800; easing.type: Easing.InOutSine }
                NumberAnimation { to: 1.0; duration: 800; easing.type: Easing.InOutSine }
            }
        }

        Label {
            text: pill.label
            color: pill.tone
            font.pixelSize: 13
            font.weight: Font.DemiBold
        }
    }
}
