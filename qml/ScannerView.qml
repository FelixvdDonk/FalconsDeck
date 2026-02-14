import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Button {
                text: bleScanner.scanning ? "Stop Scan" : "Start Scan"
                font.pixelSize: 18
                Layout.preferredHeight: 60
                Layout.preferredWidth: 200
                onClicked: {
                    if (bleScanner.scanning) {
                        bleScanner.stopScan()
                    } else {
                        bleScanner.startScan()
                    }
                }
            }

            Label {
                text: bleScanner.scanning ? "Scanning..." : "Ready"
                font.pixelSize: 16
                Layout.fillWidth: true
            }

            Label {
                text: "Found: " + deviceListView.count + " devices"
                font.pixelSize: 16
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#2a2a2a"
            border.color: "#444444"
            border.width: 2
            radius: 5

            ListView {
                id: deviceListView
                anchors.fill: parent
                anchors.margins: 5
                clip: true
                spacing: 5

                model: bleScanner.discoveredDevices

                delegate: Rectangle {
                    width: deviceListView.width - 10
                    height: 80
                    color: "#3a3a3a"
                    border.color: "#555555"
                    border.width: 1
                    radius: 5

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 15

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 5

                            Label {
                                text: modelData.name
                                font.pixelSize: 18
                                font.bold: true
                                color: "white"
                                Layout.fillWidth: true
                            }

                            Label {
                                text: "Address: " + modelData.address
                                font.pixelSize: 14
                                color: "#cccccc"
                            }

                            Label {
                                text: "RSSI: " + modelData.rssi + " dBm"
                                font.pixelSize: 14
                                color: modelData.rssi > -70 ? "#00ff00" : 
                                      modelData.rssi > -85 ? "#ffaa00" : "#ff0000"
                            }
                        }

                        Button {
                            text: "Connect"
                            font.pixelSize: 16
                            Layout.preferredWidth: 120
                            Layout.preferredHeight: 50
                            onClicked: {
                                console.log("Connecting to device:", modelData.address)
                                var device = bleScanner.getDeviceByAddress(modelData.address)
                                if (device.address.toString() !== "00:00:00:00:00:00") {
                                    connectionManager.connectRobot(device)
                                    bleScanner.stopScan()
                                }
                            }
                        }
                    }
                }

                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AsNeeded
                }
            }

            Label {
                anchors.centerIn: parent
                text: "No devices found.\nPress 'Start Scan' to discover BLE devices."
                font.pixelSize: 16
                color: "#888888"
                horizontalAlignment: Text.AlignHCenter
                visible: deviceListView.count === 0 && !bleScanner.scanning
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: bleScanner.scanning && deviceListView.count === 0
                width: 100
                height: 100
            }
        }
    }
}
