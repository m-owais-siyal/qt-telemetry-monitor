import QtQuick
import QtQuick.Controls.Basic

TextField {
    id: control

    implicitHeight: 42
    leftPadding: 12
    rightPadding: 12
    color: enabled ? Theme.textPrimary : Theme.textSecondary
    placeholderTextColor: Theme.textMuted
    selectionColor: Theme.accent
    selectedTextColor: "#ffffff"
    font.pixelSize: 14
    font.features: { "tnum": 1 }

    background: Rectangle {
        radius: Theme.radiusSmall
        color: control.enabled ? Theme.background : Theme.surface
        border.color: control.activeFocus ? Theme.accent : Theme.border
        border.width: control.activeFocus ? 2 : 1

        Behavior on border.color { ColorAnimation { duration: 120 } }
    }
}
