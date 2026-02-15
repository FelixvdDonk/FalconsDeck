import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 1280
    height: 800
    visible: true
    title: "FalconsDeck"
    color: "#121212"

    header: ToolBar {
        background: Rectangle {
            color: "#1e1e1e"
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: "#333333"
            }
        }
        height: 56

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            spacing: 16

            Label {
                text: "⚡ FalconsDeck"
                font.pixelSize: 22
                font.bold: true
                color: "#ffffff"
            }

            Item { Layout.fillWidth: true }

            Button {
                id: scannerTab
                text: "🔍 Scanner"
                font.pixelSize: 14
                flat: true
                checkable: true
                checked: swipeView.currentIndex === 0
                onClicked: swipeView.currentIndex = 0
                contentItem: Label {
                    text: scannerTab.text
                    font: scannerTab.font
                    color: scannerTab.checked ? "#4fc3f7" : "#888888"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: scannerTab.checked ? "#1a3a4a" : (scannerTab.hovered ? "#2a2a2a" : "transparent")
                    radius: 6
                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 3
                        color: "#4fc3f7"
                        visible: scannerTab.checked
                        radius: 1
                    }
                }
            }

            Button {
                id: dashboardTab
                text: "📊 Dashboard"
                font.pixelSize: 14
                flat: true
                checkable: true
                checked: swipeView.currentIndex === 1
                onClicked: swipeView.currentIndex = 1
                contentItem: Label {
                    text: dashboardTab.text
                    font: dashboardTab.font
                    color: dashboardTab.checked ? "#4fc3f7" : "#888888"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: dashboardTab.checked ? "#1a3a4a" : (dashboardTab.hovered ? "#2a2a2a" : "transparent")
                    radius: 6
                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 3
                        color: "#4fc3f7"
                        visible: dashboardTab.checked
                        radius: 1
                    }
                }
            }

            Rectangle {
                Layout.preferredWidth: connectedLabel.implicitWidth + 24
                Layout.preferredHeight: 32
                radius: 16
                color: connectionManager.connectedCount > 0 ? "#1b5e20" : "#333333"
                border.color: connectionManager.connectedCount > 0 ? "#4caf50" : "#555555"
                border.width: 1

                Label {
                    id: connectedLabel
                    anchors.centerIn: parent
                    text: connectionManager.connectedCount + " connected"
                    font.pixelSize: 13
                    color: connectionManager.connectedCount > 0 ? "#81c784" : "#888888"
                }
            }
        }
    }

    SwipeView {
        id: swipeView
        anchors.fill: parent
        interactive: false

        ScannerView {}
        RobotDashboard {}
    }

    Connections {
        target: connectionManager
        function onRobotConnected(index) {
            if (connectionManager.connectedCount === 1) {
                swipeView.currentIndex = 1
            }
        }
    }
}
