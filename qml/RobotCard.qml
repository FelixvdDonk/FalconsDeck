import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property string robotName: "Robot"
    property string robotAddress: "00:00:00:00:00:00"
    property string connectionState: "Disconnected"
    property int rssi: -100
    property string lastData: ""
    property string lastDataTime: ""
    property int robotIndex: 0
    property string deviceType: "Unknown"

    // BMS data
    property real totalVoltage: 0.0
    property real current: 0.0
    property int soc: 0
    property var cellVoltages: []

    // Falcons Robot data
    property int playState: 0
    property string playStateLabel: "Off"
    property string wifiSsid: ""
    property var wifiList: []
    property real batteryVoltage: 0.0
    property string robotIdentity: ""

    signal disconnectClicked()
    signal playStateChangeRequested(int newState)
    signal wifiSsidChangeRequested(string ssid)

    readonly property bool isFalconsRobot: deviceType === "FalconsRobot"
    readonly property bool isSmartBMS: deviceType === "SmartBMS"

    readonly property color stateColor: {
        switch (connectionState) {
            case "Ready":      return "#4caf50"
            case "Connected":  return "#4caf50"
            case "Connecting": return "#2196f3"
            case "Error":      return "#f44336"
            default:           return "#555555"
        }
    }

    readonly property color playStateColor: {
        switch (playState) {
            case 4:  return "#4caf50"  // INPLAY - green
            case 3:  return "#ff9800"  // KICKER_ON - orange
            case 2:  return "#ff9800"  // MOTOR_ON - orange
            case 1:  return "#2196f3"  // SOFTWARE_ON - blue
            default: return "#555555"  // OFF - gray
        }
    }

    color: "#1e1e1e"
    border.color: isFalconsRobot ? playStateColor : stateColor
    border.width: 2
    radius: 12

    // ── Disconnect button pinned to bottom ──
    Button {
        id: disconnectBtn
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 16
        height: 40
        visible: connectionState !== "Disconnected"
        onClicked: root.disconnectClicked()
        z: 1

        contentItem: Label {
            text: "✕  Disconnect"
            font.pixelSize: 14
            color: "white"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        background: Rectangle {
            color: parent.hovered ? "#c62828" : "#b71c1c"
            radius: 8
            opacity: parent.hovered ? 1.0 : 0.85
        }
    }

    // ── Scrollable content area ──
    Flickable {
        id: contentFlick
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: disconnectBtn.visible ? disconnectBtn.top : parent.bottom
        anchors.margins: 16
        anchors.bottomMargin: disconnectBtn.visible ? 8 : 16
        contentHeight: contentCol.implicitHeight
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.VerticalFlick

        ScrollBar.vertical: ScrollBar {
            policy: contentFlick.contentHeight > contentFlick.height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
            width: 4
        }

        ColumnLayout {
            id: contentCol
            width: contentFlick.width - 8
            spacing: 10

            // ── Header: name + status dot ──
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Rectangle {
                    width: 10; height: 10; radius: 5
                    color: root.stateColor

                    SequentialAnimation on opacity {
                        running: connectionState === "Connecting"
                        loops: Animation.Infinite
                        NumberAnimation { to: 0.3; duration: 600 }
                        NumberAnimation { to: 1.0; duration: 600 }
                    }
                }

                Label {
                    text: {
                        if (isFalconsRobot && robotIdentity)
                            return robotIdentity
                        return robotName || "Device " + (robotIndex + 1)
                    }
                    font.pixelSize: 18
                    font.bold: true
                    color: "white"
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }

                // Device type badge
                Rectangle {
                    visible: isFalconsRobot || isSmartBMS
                    Layout.preferredWidth: typeBadgeLabel.implicitWidth + 12
                    Layout.preferredHeight: 22
                    radius: 11
                    color: isFalconsRobot ? "#1a237e" : "#004d40"

                    Label {
                        id: typeBadgeLabel
                        anchors.centerIn: parent
                        text: isFalconsRobot ? "Robot" : "BMS"
                        font.pixelSize: 10
                        font.bold: true
                        color: "white"
                    }
                }

                Label {
                    text: rssi + " dBm"
                    font.pixelSize: 12
                    color: rssi > -70 ? "#4caf50" :
                           rssi > -85 ? "#ff9800" : "#f44336"
                    visible: connectionState === "Connected" || connectionState === "Ready"
                }
            }

            // ── Address ──
            Label {
                text: robotAddress
                font.pixelSize: 11
                font.family: "monospace"
                color: "#777777"
                Layout.fillWidth: true
            }

            // ── Separator ──
            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: "#333333"
            }

            // ══════════════════════════════════════════════════
            // ── FALCONS ROBOT UI ──
            // ══════════════════════════════════════════════════
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 10
                visible: isFalconsRobot && connectionState === "Ready"

                // ── Play State Display & Control ──
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    Label {
                        text: "Play State"
                        font.pixelSize: 12
                        color: "#aaaaaa"
                    }

                    // Current state display
                    Rectangle {
                        Layout.fillWidth: true
                        height: 44
                        radius: 8
                        color: root.playStateColor
                        opacity: 0.9

                        Label {
                            anchors.centerIn: parent
                            text: playStateLabel
                            font.pixelSize: 18
                            font.bold: true
                            color: "white"
                        }

                        Behavior on color {
                            ColorAnimation { duration: 300 }
                        }
                    }

                    // Play state buttons
                    GridLayout {
                        Layout.fillWidth: true
                        columns: 3
                        rowSpacing: 4
                        columnSpacing: 4

                        Repeater {
                            model: [
                                { label: "Off",     state: 0, color: "#555555" },
                                { label: "SW On",   state: 1, color: "#1565c0" },
                                { label: "Motors",  state: 2, color: "#e65100" },
                                { label: "Kicker",  state: 3, color: "#e65100" },
                                { label: "\u25B6 PLAY",  state: 4, color: "#2e7d32" }
                            ]

                            Button {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 32
                                enabled: playState !== modelData.state

                                contentItem: Label {
                                    text: modelData.label
                                    font.pixelSize: 11
                                    font.bold: modelData.state === 4
                                    color: enabled ? "white" : "#888888"
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }

                                background: Rectangle {
                                    color: enabled ? (parent.hovered ? Qt.lighter(modelData.color, 1.2) : modelData.color)
                                                   : "#2a2a2a"
                                    radius: 6
                                    border.color: playState === modelData.state ? "white" : "transparent"
                                    border.width: playState === modelData.state ? 2 : 0
                                }

                                onClicked: root.playStateChangeRequested(modelData.state)
                            }
                        }
                    }
                }

                // ── Separator ──
                Rectangle { Layout.fillWidth: true; height: 1; color: "#333333" }

                // ── Battery Voltage ──
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    visible: batteryVoltage > 0

                    Label {
                        text: "Battery"
                        font.pixelSize: 13
                        color: "#aaaaaa"
                    }
                    Item { Layout.fillWidth: true }
                    Label {
                        text: batteryVoltage.toFixed(1) + " V"
                        font.pixelSize: 20
                        font.bold: true
                        color: batteryVoltage > 28 ? "#4caf50" :
                               batteryVoltage > 25 ? "#ff9800" : "#f44336"
                    }
                }

                // ── Separator ──
                Rectangle {
                    Layout.fillWidth: true; height: 1; color: "#333333"
                    visible: batteryVoltage > 0
                }

                // ── WiFi SSID Display & Control ──
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    Label {
                        text: "WiFi"
                        font.pixelSize: 13
                        color: "#aaaaaa"
                    }

                    // Current SSID
                    Rectangle {
                        Layout.fillWidth: true
                        height: 32
                        radius: 6
                        color: "#252525"

                        Label {
                            anchors.centerIn: parent
                            text: wifiSsid || "Not connected"
                            font.pixelSize: 14
                            font.family: "monospace"
                            color: wifiSsid ? "#4fc3f7" : "#666666"
                        }
                    }

                    // Available WiFi networks (collapsible)
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4
                        visible: root.wifiList.length > 0

                        Label {
                            text: "Available networks (" + root.wifiList.length + ")"
                            font.pixelSize: 11
                            color: "#888888"

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: wifiListExpanded.visible = !wifiListExpanded.visible
                            }
                        }

                        ColumnLayout {
                            id: wifiListExpanded
                            Layout.fillWidth: true
                            spacing: 2
                            visible: false

                            Repeater {
                                model: root.wifiList

                                Button {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 28
                                    enabled: modelData !== wifiSsid

                                    contentItem: Label {
                                        text: modelData
                                        font.pixelSize: 12
                                        color: modelData === wifiSsid ? "#4fc3f7" : "white"
                                        horizontalAlignment: Text.AlignLeft
                                        verticalAlignment: Text.AlignVCenter
                                        leftPadding: 8
                                        elide: Text.ElideRight
                                    }

                                    background: Rectangle {
                                        color: modelData === wifiSsid ? "#1a3a4a"
                                             : parent.hovered ? "#333333" : "#252525"
                                        radius: 4
                                        border.color: modelData === wifiSsid ? "#4fc3f7" : "transparent"
                                        border.width: 1
                                    }

                                    onClicked: root.wifiSsidChangeRequested(modelData)
                                }
                            }
                        }
                    }
                }
            }

            // ══════════════════════════════════════════════════
            // ── SMART BMS UI (existing) ──
            // ══════════════════════════════════════════════════
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8
                visible: isSmartBMS && totalVoltage > 0

                // Large voltage display
                Label {
                    text: totalVoltage.toFixed(2) + " V"
                    font.pixelSize: 32
                    font.bold: true
                    color: "#4caf50"
                    Layout.alignment: Qt.AlignHCenter
                }

                // SoC bar
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    RowLayout {
                        Layout.fillWidth: true
                        Label {
                            text: "Battery"
                            font.pixelSize: 12
                            color: "#aaaaaa"
                        }
                        Item { Layout.fillWidth: true }
                        Label {
                            text: soc + "%"
                            font.pixelSize: 14
                            font.bold: true
                            color: soc > 50 ? "#4caf50" :
                                   soc > 20 ? "#ff9800" : "#f44336"
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 10
                        radius: 5
                        color: "#333333"

                        Rectangle {
                            width: parent.width * Math.max(0, Math.min(soc, 100)) / 100
                            height: parent.height
                            radius: 5
                            color: soc > 50 ? "#4caf50" :
                                   soc > 20 ? "#ff9800" : "#f44336"

                            Behavior on width {
                                NumberAnimation { duration: 400; easing.type: Easing.OutCubic }
                            }
                        }
                    }
                }

                // Current
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Label {
                        text: "Current"
                        font.pixelSize: 12
                        color: "#aaaaaa"
                    }
                    Item { Layout.fillWidth: true }
                    Label {
                        text: (current >= 0 ? "+" : "") + current.toFixed(2) + " A"
                        font.pixelSize: 14
                        font.bold: true
                        color: current > 0.1 ? "#4caf50" :
                               current < -0.1 ? "#f44336" : "#888888"
                    }
                }

                // ── Cell Voltages ──
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    visible: root.cellVoltages.length > 0

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: "#333333"
                    }

                    Label {
                        text: "Cell Voltages"
                        font.pixelSize: 12
                        color: "#aaaaaa"
                    }

                    Repeater {
                        model: root.cellVoltages.length

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            required property int index

                            Label {
                                text: "C" + (index + 1)
                                font.pixelSize: 11
                                color: "#888888"
                                Layout.preferredWidth: 24
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                height: 14
                                radius: 3
                                color: "#252525"

                                Rectangle {
                                    property real cellV: root.cellVoltages[index] || 0
                                    property real pct: Math.max(0, Math.min(100, (cellV - 2.5) / 1.7 * 100))
                                    width: parent.width * pct / 100
                                    height: parent.height
                                    radius: 3
                                    color: pct > 60 ? "#4caf50" :
                                           pct > 25 ? "#ff9800" : "#f44336"

                                    Behavior on width {
                                        NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
                                    }
                                }
                            }

                            Label {
                                text: (root.cellVoltages[index] || 0).toFixed(3) + "V"
                                font.pixelSize: 11
                                font.family: "monospace"
                                font.bold: true
                                color: "#cccccc"
                                Layout.preferredWidth: 54
                                horizontalAlignment: Text.AlignRight
                            }
                        }
                    }
                }
            }

            // ── Waiting for data state ──
            Label {
                text: connectionState === "Connecting" ? "Connecting\u2026" :
                      connectionState === "Connected" || connectionState === "Ready"
                          ? (isFalconsRobot ? "Waiting for robot data\u2026" : "Waiting for data\u2026") : ""
                font.pixelSize: 14
                color: "#666666"
                Layout.alignment: Qt.AlignHCenter
                visible: {
                    if (connectionState === "Disconnected" || connectionState === "Error")
                        return false
                    if (isFalconsRobot)
                        return connectionState !== "Ready"
                    return totalVoltage === 0
                }
            }

            // ── Error state ──
            Label {
                text: "\u26A0 Connection error"
                font.pixelSize: 14
                color: "#f44336"
                Layout.alignment: Qt.AlignHCenter
                visible: connectionState === "Error"
            }
        }
    }
}
