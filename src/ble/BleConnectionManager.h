#ifndef BLECONNECTIONMANAGER_H
#define BLECONNECTIONMANAGER_H

#include <QObject>
#include <QVector>
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
    void connectRobot(const QBluetoothDeviceInfo &device);
    void disconnectRobot(int index);
    void disconnectRobotByAddress(const QString &address);
    void disconnectAll();
    void sendToRobot(int index, const QByteArray &data);
    void sendToAll(const QByteArray &data);

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

    QVector<BleRobotConnection*> m_connections;
    RobotListModel *m_robotListModel;
    int m_connectedCount;
    int m_nextRobotId;
};

#endif // BLECONNECTIONMANAGER_H
