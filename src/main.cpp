#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QBluetoothDeviceInfo>
#include "src/ble/BleDeviceScanner.h"
#include "src/ble/BleConnectionManager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("FalconsDeck");
    app.setOrganizationName("Falcons");
    app.setApplicationVersion("1.0.0");

    // Register metatypes for QML
    qRegisterMetaType<QBluetoothDeviceInfo>("QBluetoothDeviceInfo");

    // Create BLE components
    BleDeviceScanner scanner;
    BleConnectionManager connectionManager;
    connectionManager.setScanner(&scanner);

    QQmlApplicationEngine engine;

    // Expose objects to QML
    engine.rootContext()->setContextProperty("bleScanner", &scanner);
    engine.rootContext()->setContextProperty("connectionManager", &connectionManager);

    const QUrl url(QStringLiteral("qrc:/qt/qml/FalconsDeck/qml/Main.qml"));
    
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);

    // Auto-start BLE scanning
    scanner.startScan();

    return app.exec();
}
