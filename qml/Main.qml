import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 1280
    height: 800
    visible: true
    title: "FalconsDeck - Robot Monitor"

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.margins: 5

            Label {
                text: "FalconsDeck"
                font.pixelSize: 20
                font.bold: true
                Layout.fillWidth: true
            }

            Label {
                text: "Connected: " + connectionManager.connectedCount + " / 16"
                font.pixelSize: 16
            }

            Button {
                text: stackView.currentItem.title || "Scanner"
                onClicked: {
                    if (stackView.currentItem === scannerView) {
                        stackView.push(dashboardView)
                    } else {
                        stackView.push(scannerView)
                    }
                }
            }
        }
    }

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: scannerView

        Component {
            id: scannerView
            ScannerView {
                property string title: "Scanner"
            }
        }

        Component {
            id: dashboardView
            RobotDashboard {
                property string title: "Dashboard"
            }
        }
    }
}
