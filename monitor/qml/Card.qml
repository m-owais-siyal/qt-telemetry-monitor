import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

// A rounded surface with an optional small-caps title. Children are laid out in a column.
Rectangle {
    id: card

    property string title
    default property alias content: body.data
    property alias headerItems: headerExtras.data

    implicitHeight: layout.implicitHeight + 2 * Theme.padding
    color: Theme.surface
    radius: Theme.radius
    border.color: Theme.border
    border.width: 1

    ColumnLayout {
        id: layout

        anchors.fill: parent
        anchors.margins: Theme.padding
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            visible: card.title.length > 0

            Label {
                text: card.title.toUpperCase()
                color: Theme.textMuted
                font.pixelSize: 11
                font.weight: Font.DemiBold
                font.letterSpacing: 1.2
            }

            Item { Layout.fillWidth: true }

            Row {
                id: headerExtras
                spacing: 8
            }
        }

        ColumnLayout {
            id: body

            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8
        }
    }
}
