import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // Top bar: scan controls
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Button {
                text: bleScanner.scanning ? "⏹ Stop Scan" : "▶ Start Scan"
                font.pixelSize: 16
                font.bold: true
                Layout.preferredHeight: 48
                Layout.preferredWidth: 180
                onClicked: {
                    if (bleScanner.scanning) {
                        bleScanner.stopScan()
                    } else {
                        bleScanner.startScan()
                    }
                }
                contentItem: Label {
                    text: parent.text
                    font: parent.font
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: bleScanner.scanning ? "#c62828" : "#2e7d32"
                    radius: 8
                    opacity: parent.hovered ? 0.85 : 1.0
                }
            }

            CheckBox {
                id: filterCheckbox
                text: "SmartBMS only"
                checked: bleScanner.filterEnabled
                font.pixelSize: 14
                onCheckedChanged: bleScanner.filterEnabled = checked
                contentItem: Label {
                    text: filterCheckbox.text
                    font: filterCheckbox.font
                    color: "#cccccc"
                    leftPadding: filterCheckbox.indicator.width + filterCheckbox.spacing
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Item { Layout.fillWidth: true }

            BusyIndicator {
                running: bleScanner.scanning
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
                visible: bleScanner.scanning
            }

            Label {
                text: deviceListView.count + " devices found"
                font.pixelSize: 14
                color: "#aaaaaa"
            }
        }

        // Device list
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#1a1a1a"
            border.color: "#333333"
            border.width: 1
            radius: 8

            ListView {
                id: deviceListView
                anchors.fill: parent
                anchors.margins: 8
                clip: true
                spacing: 6

                model: bleScanner.discoveredDevices

                delegate: Rectangle {
                    width: deviceListView.width
                    height: 72
                    color: mouseArea.containsMouse ? "#333333" : "#252525"
                    border.color: "#444444"
                    border.width: 1
                    radius: 8

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 16
                        anchors.rightMargin: 12
                        spacing: 16

                        // Signal strength indicator
                        Rectangle {
                            Layout.preferredWidth: 48
                            Layout.preferredHeight: 48
                            radius: 24
                            color: modelData.rssi > -70 ? "#1b5e20" :
                                   modelData.rssi > -85 ? "#e65100" : "#b71c1c"

                            Label {
                                anchors.centerIn: parent
                                text: modelData.rssi
                                font.pixelSize: 12
                                font.bold: true
                                color: "white"
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2

                            Label {
                                text: modelData.name || "Unknown"
                                font.pixelSize: 16
                                font.bold: true
                                color: "white"
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }

                            Label {
                                text: modelData.address
                                font.pixelSize: 12
                                color: "#999999"
                                font.family: "monospace"
                            }
                        }

                        Button {
                            text: "Connect"
                            font.pixelSize: 14
                            Layout.preferredWidth: 110
                            Layout.preferredHeight: 40
                            contentItem: Label {
                                text: parent.text
                                font: parent.font
                                color: "white"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            background: Rectangle {
                                color: parent.hovered ? "#1565c0" : "#1976d2"
                                radius: 6
                            }
                            onClicked: {
                                console.log("Connecting to device:", modelData.address)
                                var device = bleScanner.getDeviceByAddress(modelData.address)
                                connectionManager.connectRobot(device)
                                bleScanner.stopScan()
                            }
                        }
                    }
                }

                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AsNeeded
                }
            }

            // Empty state
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 12
                visible: deviceListView.count === 0 && !bleScanner.scanning

                Label {
                    text: "📡"
                    font.pixelSize: 48
                    Layout.alignment: Qt.AlignHCenter
                }

                Label {
                    text: bleScanner.filterEnabled
                        ? "No SmartBMS devices found"
                        : "No devices found"
                    font.pixelSize: 18
                    color: "#888888"
                    Layout.alignment: Qt.AlignHCenter
                }

                Label {
                    text: "Press 'Start Scan' to search for BLE devices"
                    font.pixelSize: 14
                    color: "#666666"
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }
    }
}
