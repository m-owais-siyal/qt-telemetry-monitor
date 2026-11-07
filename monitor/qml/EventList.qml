import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import TelemetryMonitor

ListView {
    id: list

    clip: true
    spacing: 2
    boundsBehavior: Flickable.StopAtBounds

    ScrollBar.vertical: ScrollBar {
        policy: ScrollBar.AsNeeded
    }

    add: Transition {
        NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 200 }
    }

    // Slide existing rows down when new ones arrive. Animating opacity back to 1
    // here matters: if an add transition is interrupted by the next insert, the
    // row would otherwise be left half-transparent.
    displaced: Transition {
        NumberAnimation { property: "y"; duration: 200; easing.type: Easing.OutCubic }
        NumberAnimation { property: "opacity"; to: 1; duration: 200 }
    }

    delegate: Item {
        id: entry

        required property string time
        required property int severity
        required property string message

        readonly property color tone: {
            switch (severity) {
            case EventLogModel.Success: return Theme.success
            case EventLogModel.Warning: return Theme.warning
            case EventLogModel.Error: return Theme.danger
            default: return Theme.accent
            }
        }

        width: ListView.view.width
        implicitHeight: row.implicitHeight + 10

        RowLayout {
            id: row

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            spacing: 10

            Rectangle {
                Layout.alignment: Qt.AlignVCenter
                implicitWidth: 6
                implicitHeight: 6
                radius: 3
                color: entry.tone
            }

            Label {
                text: entry.time
                color: Theme.textMuted
                font.pixelSize: 12
                font.features: { "tnum": 1 }
            }

            Label {
                Layout.fillWidth: true
                text: entry.message
                color: Theme.textSecondary
                font.pixelSize: 13
                elide: Text.ElideRight
            }
        }
    }

    Label {
        anchors.centerIn: parent
        visible: list.count === 0
        text: qsTr("No events yet")
        color: Theme.textMuted
        font.pixelSize: 13
    }
}
