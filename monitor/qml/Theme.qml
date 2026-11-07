pragma Singleton

import QtQuick

QtObject {
    readonly property color background: "#0b0e14"
    readonly property color surface: "#121722"
    readonly property color surfaceRaised: "#1b2232"
    readonly property color border: "#222b3b"
    readonly property color gridLine: "#1a2230"

    readonly property color textPrimary: "#e8ebf1"
    readonly property color textSecondary: "#97a0b3"
    readonly property color textMuted: "#5f6a7f"

    readonly property color accent: "#5b8cff"
    readonly property color accentAlt: "#9b6bff"
    readonly property color success: "#3ecf8e"
    readonly property color warning: "#f5b14c"
    readonly property color danger: "#ff6b6b"

    // One colour per telemetry channel: temperature, pressure, voltage.
    readonly property var channelColors: ["#ff8a5c", "#4fc3f7", "#b388ff"]

    readonly property int radius: 12
    readonly property int radiusSmall: 8
    readonly property int spacing: 20
    readonly property int padding: 18

    function withAlpha(base: color, alpha: real): color {
        return Qt.rgba(base.r, base.g, base.b, alpha)
    }
}
