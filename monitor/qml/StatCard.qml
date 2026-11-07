import QtQuick
import QtQuick.Controls.Basic

Rectangle {
    id: card

    property string label
    property string value
    property string caption
    property color valueColor: Theme.textPrimary

    implicitHeight: 108
    color: Theme.surface
    radius: Theme.radius
    border.color: Theme.border
    border.width: 1

    Column {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: Theme.padding
        anchors.rightMargin: Theme.padding
        anchors.verticalCenter: parent.verticalCenter
        spacing: 6

        Label {
            text: card.label.toUpperCase()
            color: Theme.textMuted
            font.pixelSize: 11
            font.weight: Font.DemiBold
            font.letterSpacing: 1.2
        }

        Label {
            text: card.value
            color: card.valueColor
            font.pixelSize: 30
            font.weight: Font.Bold
            font.features: { "tnum": 1 } // tabular digits, so numbers don't jitter as they change

            Behavior on color { ColorAnimation { duration: 200 } }
        }

        Label {
            width: parent.width
            text: card.caption
            visible: text.length > 0
            color: Theme.textSecondary
            font.pixelSize: 12
            elide: Text.ElideRight
        }
    }
}
