#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include "src/ble/BleDeviceScanner.h"
#include "src/ble/BleConnectionManager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("FalconsDeck");
    app.setOrganizationName("Falcons");
    app.setApplicationVersion("1.0.0");

    // Create BLE components
    BleDeviceScanner scanner;
    BleConnectionManager connectionManager;

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

    return app.exec();
}
