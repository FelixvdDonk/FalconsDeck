#ifndef BLECONNECTIONMANAGER_H
#define BLECONNECTIONMANAGER_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QBluetoothDeviceInfo>
#include "BleRobotConnection.h"
#include "src/models/RobotListModel.h"
#include "src/models/Robot.h"

class BleConnectionManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(RobotListModel* robotListModel READ robotListModel CONSTANT)
    Q_PROPERTY(int connectedCount READ connectedCount NOTIFY connectedCountChanged)

public:
    static const int MAX_ROBOTS = 16;

    explicit BleConnectionManager(QObject *parent = nullptr);
    ~BleConnectionManager();

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

signals:
    void connectedCountChanged();
    void robotConnected(int index);
    void robotDisconnected(int index);
    void robotError(int index, const QString &error);

private slots:
    void onConnectionStateChanged();
    void onDataReceived(const QByteArray &data);
    void onErrorOccurred(const QString &error);

private:
    int findConnectionIndex(BleRobotConnection *connection);
    int findConnectionByAddress(const QString &address);
    void updateConnectedCount();
    void updateRobotModel(int index);

    QList<BleRobotConnection*> m_connections;
    RobotListModel *m_robotListModel;
    int m_connectedCount;
    int m_nextRobotId;
};

#endif // BLECONNECTIONMANAGER_H
