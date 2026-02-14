#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QBluetoothDeviceInfo>
#include <QQuickWindow>
#include "src/ble/BleDeviceScanner.h"
#include "src/ble/BleConnectionManager.h"

int main(int argc, char *argv[])
{
    // Reduce input latency: use threaded render loop so input is processed
    // on the main thread independently of the render thread's vsync wait.
    qputenv("QSG_RENDER_LOOP", "threaded");

    // Minimize idle-to-render delay
    qputenv("QT_QPA_UPDATE_IDLE_TIME", "0");

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
