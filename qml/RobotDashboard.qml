import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // Control panel
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 100
            color: "#2a2a2a"
            border.color: "#444444"
            border.width: 2
            radius: 5

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                Label {
                    text: "Send Data to Robots"
                    font.pixelSize: 16
                    font.bold: true
                    color: "white"
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    TextField {
                        id: dataInput
                        Layout.fillWidth: true
                        placeholderText: "Enter hex data (e.g., 01 02 03 04) or text"
                        font.pixelSize: 14
                    }

                    Button {
                        text: "Send to Selected"
                        font.pixelSize: 14
                        enabled: dataInput.text.length > 0 && robotGrid.currentIndex >= 0
                        onClicked: {
                            var data = parseInput(dataInput.text)
                            if (data.length > 0) {
                                connectionManager.sendToRobot(robotGrid.currentIndex, data)
                                console.log("Sent to robot", robotGrid.currentIndex, ":", data.toString('hex'))
                            }
                        }
                    }

                    Button {
                        text: "Send to All"
                        font.pixelSize: 14
                        enabled: dataInput.text.length > 0 && connectionManager.connectedCount > 0
                        onClicked: {
                            var data = parseInput(dataInput.text)
                            if (data.length > 0) {
                                connectionManager.sendToAll(data)
                                console.log("Broadcast:", data.toString('hex'))
                            }
                        }
                    }
                }
            }
        }

        // Robot grid (4x4 layout for 16 robots)
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#1a1a1a"
            border.color: "#444444"
            border.width: 2
            radius: 5

            GridView {
                id: robotGrid
                anchors.fill: parent
                anchors.margins: 10
                cellWidth: width / 4
                cellHeight: height / 4
                clip: true

                model: connectionManager.robotListModel

                delegate: Item {
                    width: robotGrid.cellWidth
                    height: robotGrid.cellHeight

                    RobotCard {
                        anchors.fill: parent
                        anchors.margins: 5
                        robotName: model.name
                        robotAddress: model.address
                        connectionState: model.connectionState
                        rssi: model.rssi
                        lastData: model.lastData
                        lastDataTime: model.lastDataTime
                        robotIndex: index

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                robotGrid.currentIndex = index
                            }
                        }

                        // Highlight selected robot
                        border.width: robotGrid.currentIndex === index ? 4 : 2

                        onDisconnectClicked: {
                            connectionManager.disconnectRobot(index)
                        }
                    }
                }

                // Show placeholder for empty slots
                Repeater {
                    model: 16 - robotGrid.count
                    
                    Rectangle {
                        x: (robotGrid.count + index) % 4 * robotGrid.cellWidth + 5
                        y: Math.floor((robotGrid.count + index) / 4) * robotGrid.cellHeight + 5
                        width: robotGrid.cellWidth - 10
                        height: robotGrid.cellHeight - 10
                        color: "#2a2a2a"
                        border.color: "#333333"
                        border.width: 1
                        radius: 8

                        Label {
                            anchors.centerIn: parent
                            text: "Slot " + (robotGrid.count + index + 1)
                            font.pixelSize: 14
                            color: "#666666"
                        }
                    }
                }
            }

            Label {
                anchors.centerIn: parent
                text: "No robots connected.\nUse the Scanner to connect to robots."
                font.pixelSize: 16
                color: "#888888"
                horizontalAlignment: Text.AlignHCenter
                visible: robotGrid.count === 0
            }
        }
    }

    // Helper function to parse input as hex or text
    function parseInput(input) {
        // Remove whitespace
        var cleaned = input.replace(/\s+/g, '')
        
        // Check if it looks like hex (pairs of hex digits)
        if (/^[0-9a-fA-F]+$/.test(cleaned) && cleaned.length % 2 === 0) {
            // Parse as hex
            var bytes = []
            for (var i = 0; i < cleaned.length; i += 2) {
                bytes.push(parseInt(cleaned.substr(i, 2), 16))
            }
            return bytes
        } else {
            // Parse as text (UTF-8)
            var encoder = new TextEncoder()
            return Array.from(encoder.encode(input))
        }
    }
}
