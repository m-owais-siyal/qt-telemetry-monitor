pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

// A row of mutually exclusive options, e.g. 10 s / 30 s / 60 s.
Rectangle {
    id: control

    property var options: []
    property int currentValue
    property string suffix

    signal activated(int value)

    implicitHeight: 38
    radius: Theme.radiusSmall
    color: Theme.background
    border.color: Theme.border
    border.width: 1

    RowLayout {
        anchors.fill: parent
        anchors.margins: 3
        spacing: 3

        Repeater {
            model: control.options

            delegate: Rectangle {
                id: segment

                required property int modelData
                readonly property bool selected: modelData === control.currentValue

                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: Theme.radiusSmall - 2
                color: selected ? Theme.surfaceRaised : hover.hovered ? Theme.withAlpha(Theme.surfaceRaised, 0.5) : "transparent"

                Behavior on color { ColorAnimation { duration: 120 } }

                Label {
                    anchors.centerIn: parent
                    text: segment.modelData + control.suffix
                    color: segment.selected ? Theme.textPrimary : Theme.textSecondary
                    font.pixelSize: 13
                    font.weight: segment.selected ? Font.DemiBold : Font.Normal
                }

                HoverHandler {
                    id: hover
                    cursorShape: Qt.PointingHandCursor
                }

                TapHandler {
                    onTapped: control.activated(segment.modelData)
                }
            }
        }
    }
}
