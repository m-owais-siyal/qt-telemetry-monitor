pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import QtGraphs

// One chart card per channel. The role properties come from ChannelModel.
Rectangle {
    id: card

    required property int index
    required property string name
    required property string unit
    required property int decimals
    required property bool hasData
    required property var value
    required property var minimum
    required property var maximum
    required property var average
    required property real rangeMin
    required property real rangeMax
    required property real tickInterval
    required property int labelDecimals

    property color lineColor: Theme.accent
    property real viewStart: 0
    property real viewEnd: 10
    property bool running: false

    readonly property alias series: line

    function format(number: var): string {
        return number === undefined || number === null ? "—" : Number(number).toFixed(card.decimals)
    }

    color: Theme.surface
    radius: Theme.radius
    border.color: Theme.border
    border.width: 1

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.padding
        spacing: 6

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Rectangle {
                implicitWidth: 10
                implicitHeight: 10
                radius: 5
                color: card.lineColor
            }

            Label {
                text: card.name
                color: Theme.textPrimary
                font.pixelSize: 15
                font.weight: Font.DemiBold
            }

            Item { Layout.fillWidth: true }

            Repeater {
                model: [
                    { caption: qsTr("min"), number: card.minimum },
                    { caption: qsTr("avg"), number: card.average },
                    { caption: qsTr("max"), number: card.maximum }
                ]

                delegate: Row {
                    id: statistic

                    required property var modelData

                    Layout.rightMargin: 10
                    spacing: 5

                    Label {
                        anchors.baseline: statisticValue.baseline
                        text: statistic.modelData.caption
                        color: Theme.textMuted
                        font.pixelSize: 12
                    }

                    Label {
                        id: statisticValue
                        text: card.format(statistic.modelData.number)
                        color: Theme.textSecondary
                        font.pixelSize: 13
                        font.features: { "tnum": 1 }
                    }
                }
            }

            Label {
                Layout.leftMargin: 6
                text: card.format(card.value)
                color: Theme.textPrimary
                font.pixelSize: 24
                font.weight: Font.Bold
                font.features: { "tnum": 1 }
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: card.unit
                color: Theme.textSecondary
                font.pixelSize: 14
            }
        }

        GraphsView {
            id: chart

            Layout.fillWidth: true
            Layout.fillHeight: true
            marginTop: 6
            marginBottom: 0
            marginLeft: 0
            marginRight: 6
            clipPlotArea: true

            theme: GraphsTheme {
                colorScheme: GraphsTheme.ColorScheme.Dark
                backgroundVisible: false
                plotAreaBackgroundVisible: false
                grid.mainColor: Theme.gridLine
                grid.subColor: Theme.gridLine
                grid.mainWidth: 1
                axisX.mainColor: Theme.border
                axisX.labelTextColor: Theme.textMuted
                axisY.mainColor: Theme.border
                axisY.labelTextColor: Theme.textMuted
                axisXLabelFont.pixelSize: 11
                axisYLabelFont.pixelSize: 11
                seriesColors: [card.lineColor]
            }

            axisX: ValueAxis {
                min: card.viewStart
                max: card.viewEnd
                tickInterval: card.viewEnd - card.viewStart <= 10 ? 2 : card.viewEnd - card.viewStart <= 30 ? 5 : 10
                labelFormat: "%.0f s"
                subTickCount: 0
                subGridVisible: false
            }

            axisY: ValueAxis {
                min: card.rangeMin
                max: card.rangeMax
                tickInterval: card.tickInterval
                labelFormat: "%." + card.labelDecimals + "f"
                subTickCount: 0
                subGridVisible: false
            }

            LineSeries {
                id: line
                color: card.lineColor
                width: 2
            }
        }
    }

    Label {
        anchors.centerIn: parent
        anchors.verticalCenterOffset: 16
        visible: !card.hasData
        text: card.running ? qsTr("Waiting for data…") : qsTr("Press Start listening to begin")
        color: Theme.textMuted
        font.pixelSize: 13
    }
}
