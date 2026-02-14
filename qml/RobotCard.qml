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

    signal disconnectClicked()

    color: {
        switch (connectionState) {
            case "Ready": return "#1a4a1a"
            case "Connected": return "#3a3a1a"
            case "Connecting": return "#1a3a3a"
            case "Error": return "#4a1a1a"
            default: return "#2a2a2a"
        }
    }
    border.color: {
        switch (connectionState) {
            case "Ready": return "#00ff00"
            case "Connected": return "#ffaa00"
            case "Connecting": return "#00aaff"
            case "Error": return "#ff0000"
            default: return "#555555"
        }
    }
    border.width: 2
    radius: 8

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 4

        // Header with robot name and state
        RowLayout {
            Layout.fillWidth: true
            spacing: 5

            Label {
                text: robotName || "Robot " + (robotIndex + 1)
                font.pixelSize: 14
                font.bold: true
                color: "white"
                Layout.fillWidth: true
                elide: Text.ElideRight
            }

            Rectangle {
                width: 12
                height: 12
                radius: 6
                color: root.border.color
            }
        }

        // Address
        Label {
            text: robotAddress
            font.pixelSize: 10
            color: "#cccccc"
            Layout.fillWidth: true
            elide: Text.ElideMiddle
        }

        // State and RSSI
        RowLayout {
            Layout.fillWidth: true
            spacing: 5

            Label {
                text: connectionState
                font.pixelSize: 11
                color: "#aaaaaa"
                Layout.fillWidth: true
            }

            Label {
                text: rssi + " dBm"
                font.pixelSize: 11
                color: rssi > -70 ? "#00ff00" : 
                      rssi > -85 ? "#ffaa00" : "#ff0000"
                visible: connectionState !== "Disconnected"
            }
        }

        // Last data received
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#1a1a1a"
            border.color: "#333333"
            border.width: 1
            radius: 4
            visible: lastData !== ""

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 4
                spacing: 2

                Label {
                    text: "Last RX:"
                    font.pixelSize: 9
                    color: "#888888"
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true

                    TextArea {
                        text: lastData
                        font.pixelSize: 9
                        font.family: "Monospace"
                        color: "#00ff00"
                        readOnly: true
                        wrapMode: TextArea.Wrap
                        background: Rectangle {
                            color: "transparent"
                        }
                    }
                }

                Label {
                    text: lastDataTime
                    font.pixelSize: 8
                    color: "#666666"
                }
            }
        }

        // Disconnect button
        Button {
            text: "Disconnect"
            font.pixelSize: 10
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            visible: connectionState !== "Disconnected"
            onClicked: root.disconnectClicked()
        }
    }
}
