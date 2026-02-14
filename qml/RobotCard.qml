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
    property real totalVoltage: 0.0
    property real current: 0.0
    property int soc: 0
    property var cellVoltages: []

    signal disconnectClicked()

    readonly property color stateColor: {
        switch (connectionState) {
            case "Ready":      return "#4caf50"
            case "Connected":  return "#4caf50"
            case "Connecting": return "#2196f3"
            case "Error":      return "#f44336"
            default:           return "#555555"
        }
    }

    color: "#1e1e1e"
    border.color: stateColor
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
                    text: robotName || "Device " + (robotIndex + 1)
                    font.pixelSize: 18
                    font.bold: true
                    color: "white"
                    Layout.fillWidth: true
                    elide: Text.ElideRight
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

            // ── BMS live data ──
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8
                visible: totalVoltage > 0

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
                text: connectionState === "Connecting" ? "Connecting…" :
                      connectionState === "Connected" || connectionState === "Ready"
                          ? "Waiting for data…" : ""
                font.pixelSize: 14
                color: "#666666"
                Layout.alignment: Qt.AlignHCenter
                visible: totalVoltage === 0 && connectionState !== "Disconnected" && connectionState !== "Error"
            }

            // ── Error state ──
            Label {
                text: "⚠ Connection error"
                font.pixelSize: 14
                color: "#f44336"
                Layout.alignment: Qt.AlignHCenter
                visible: connectionState === "Error"
            }
        }
    }
}
