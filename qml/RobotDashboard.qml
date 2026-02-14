import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // ── Header bar ──
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Label {
                text: "🔋 Connected Devices"
                font.pixelSize: 20
                font.bold: true
                color: "white"
            }

            Item { Layout.fillWidth: true }

            Label {
                text: connectionManager.connectedCount + " active"
                font.pixelSize: 14
                color: connectionManager.connectedCount > 0 ? "#4caf50" : "#888888"
            }

            Button {
                text: "Disconnect All"
                font.pixelSize: 13
                visible: connectionManager.connectedCount > 0
                contentItem: Label {
                    text: parent.text
                    font: parent.font
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: parent.hovered ? "#c62828" : "#b71c1c"
                    radius: 6
                    opacity: parent.hovered ? 1.0 : 0.85
                }
                onClicked: connectionManager.disconnectAll()
            }
        }

        // ── Device cards grid ──
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "transparent"

            GridView {
                id: deviceGrid
                anchors.fill: parent
                clip: true
                cellWidth: Math.max(280, width / Math.max(1, Math.floor(width / 320)))
                cellHeight: 340

                model: connectionManager.robotListModel

                delegate: Item {
                    width: deviceGrid.cellWidth
                    height: deviceGrid.cellHeight

                    RobotCard {
                        anchors.fill: parent
                        anchors.margins: 6
                        robotName: model.name
                        robotAddress: model.address
                        connectionState: model.connectionState
                        rssi: model.rssi
                        lastData: model.lastData
                        lastDataTime: model.lastDataTime
                        robotIndex: index
                        totalVoltage: model.totalVoltage
                        current: model.current
                        soc: model.soc

                        onDisconnectClicked: {
                            connectionManager.disconnectRobot(index)
                        }
                    }
                }

                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AsNeeded
                }
            }

            // ── Empty state ──
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 16
                visible: deviceGrid.count === 0

                Label {
                    text: "🔌"
                    font.pixelSize: 64
                    Layout.alignment: Qt.AlignHCenter
                }

                Label {
                    text: "No devices connected"
                    font.pixelSize: 20
                    color: "#888888"
                    Layout.alignment: Qt.AlignHCenter
                }

                Label {
                    text: "Go to the Scanner tab to find and connect to BMS devices"
                    font.pixelSize: 14
                    color: "#666666"
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }
    }
}
