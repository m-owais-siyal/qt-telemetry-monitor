import QtQuick
import QtQuick.Controls.Basic

Button {
    id: control

    property color tone: Theme.accent
    property bool filled: true

    implicitHeight: 42
    hoverEnabled: true
    font.pixelSize: 14
    font.weight: Font.DemiBold

    contentItem: Label {
        text: control.text
        font: control.font
        color: control.filled ? "#ffffff" : control.tone
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        radius: Theme.radiusSmall
        color: {
            if (control.filled)
                return control.down ? Qt.darker(control.tone, 1.2)
                                    : control.hovered ? Qt.lighter(control.tone, 1.12) : control.tone
            return control.down ? Theme.withAlpha(control.tone, 0.2)
                                : control.hovered ? Theme.withAlpha(control.tone, 0.12) : "transparent"
        }
        border.color: control.filled ? "transparent" : Theme.withAlpha(control.tone, 0.5)
        border.width: 1

        Behavior on color { ColorAnimation { duration: 120 } }
    }

    HoverHandler {
        cursorShape: Qt.PointingHandCursor
    }
}
