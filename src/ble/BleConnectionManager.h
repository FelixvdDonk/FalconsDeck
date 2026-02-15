#ifndef BLECONNECTIONMANAGER_H
#define BLECONNECTIONMANAGER_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QBluetoothDeviceInfo>
#include "BleRobotConnection.h"
#include "JbdBmsConnection.h"
#include "FalconsRobotConnection.h"
#include "src/models/RobotListModel.h"
#include "src/models/Robot.h"

class BleDeviceScanner;

class BleConnectionManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(RobotListModel* robotListModel READ robotListModel CONSTANT)
    Q_PROPERTY(int connectedCount READ connectedCount NOTIFY connectedCountChanged)

public:
    static const int MAX_ROBOTS = 16;

    explicit BleConnectionManager(QObject *parent = nullptr);
    ~BleConnectionManager();

    void setScanner(BleDeviceScanner *scanner) { m_scanner = scanner; }

    RobotListModel* robotListModel() { return m_robotListModel; }
    int connectedCount() const { return m_connectedCount; }

public slots:
    Q_INVOKABLE void connectRobot(const QBluetoothDeviceInfo &device);
    Q_INVOKABLE void disconnectRobot(int index);
    Q_INVOKABLE void disconnectRobotByAddress(const QString &address);
    Q_INVOKABLE void disconnectAll();
    Q_INVOKABLE void sendToRobot(int index, const QByteArray &data);
    Q_INVOKABLE void sendToRobot(int index, const QString &text);
    Q_INVOKABLE void sendToAll(const QByteArray &data);
    Q_INVOKABLE void sendToAll(const QString &text);

    /** Write play state to a specific robot (Falcons robots only) */
    Q_INVOKABLE void writePlayState(int index, int state);

    /** Write play state to all connected Falcons robots */
    Q_INVOKABLE void writePlayStateAll(int state);

    /** Write WiFi SSID to a specific robot (Falcons robots only) */
    Q_INVOKABLE void writeWifiSsid(int index, const QString &ssid);

    /** Write WiFi SSID to all connected Falcons robots */
    Q_INVOKABLE void writeWifiSsidAll(const QString &ssid);

signals:
    void connectedCountChanged();
    void robotConnected(int index);
    void robotDisconnected(int index);
    void robotError(int index, const QString &error);

private slots:
    void onConnectionStateChanged();
    void onDataReceived(const QByteArray &data);
    void onErrorOccurred(const QString &error);
    void onJbdConnectionStateChanged();
    void onJbdBmsDataUpdated();
    void onJbdErrorOccurred(const QString &error);
    void onFalconsConnectionStateChanged();
    void onFalconsRobotDataUpdated();
    void onFalconsErrorOccurred(const QString &error);

private:
    int findConnectionIndex(BleRobotConnection *connection);
    int findConnectionByAddress(const QString &address);
    int findJbdConnectionIndex(JbdBmsConnection *connection);
    int findFalconsConnectionIndex(FalconsRobotConnection *connection);
    void updateConnectedCount();
    void updateRobotModel(int index);
    void updateJbdRobotModel(int index);

    QList<BleRobotConnection*> m_connections;
    QList<JbdBmsConnection*> m_jbdConnections;
    QList<FalconsRobotConnection*> m_falconsConnections;
    RobotListModel *m_robotListModel;
    BleDeviceScanner *m_scanner;
    int m_connectedCount;
    int m_nextRobotId;
};

#endif // BLECONNECTIONMANAGER_H
