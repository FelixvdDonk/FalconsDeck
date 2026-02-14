#include "BleConnectionManager.h"
#include <QDebug>

BleConnectionManager::BleConnectionManager(QObject *parent)
    : QObject(parent)
    , m_connectedCount(0)
    , m_nextRobotId(1)
{
    m_robotListModel = new RobotListModel(this);
}

BleConnectionManager::~BleConnectionManager()
{
    disconnectAll();
}

void BleConnectionManager::connectRobot(const QBluetoothDeviceInfo &device)
{
    // Check if already connected to this device
    QString address = device.address().toString();
    if (findConnectionByAddress(address) >= 0) {
        qWarning() << "Already connected to device:" << address;
        return;
    }

    // Check if we've reached the maximum
    if (m_connections.size() >= MAX_ROBOTS) {
        qWarning() << "Maximum number of robots reached:" << MAX_ROBOTS;
        return;
    }

    qDebug() << "Connecting to robot:" << device.name() << address;

    // Create new connection
    BleRobotConnection *connection = new BleRobotConnection(this);
    
    connect(connection, &BleRobotConnection::connectionStateChanged,
            this, &BleConnectionManager::onConnectionStateChanged);
    connect(connection, &BleRobotConnection::dataReceived,
            this, &BleConnectionManager::onDataReceived);
    connect(connection, &BleRobotConnection::errorOccurred,
            this, &BleConnectionManager::onErrorOccurred);

    int index = m_connections.size();
    m_connections.append(connection);

    // Add robot to model
    Robot robot(m_nextRobotId++, device.name(), device.address());
    robot.setConnectionState(Robot::Connecting);
    robot.setRssi(device.rssi());
    m_robotListModel->addRobot(robot);

    // Start connection
    connection->connectToDevice(device);
}

void BleConnectionManager::disconnectRobot(int index)
{
    if (index < 0 || index >= m_connections.size()) {
        qWarning() << "Invalid robot index:" << index;
        return;
    }

    qDebug() << "Disconnecting robot at index:" << index;
    
    BleRobotConnection *connection = m_connections[index];
    connection->disconnect();
    
    // Remove from model and connections
    m_robotListModel->removeRobot(index);
    m_connections.removeAt(index);
    
    connection->deleteLater();
    
    updateConnectedCount();
    emit robotDisconnected(index);
}

void BleConnectionManager::disconnectRobotByAddress(const QString &address)
{
    int index = findConnectionByAddress(address);
    if (index >= 0) {
        disconnectRobot(index);
    }
}

void BleConnectionManager::disconnectAll()
{
    qDebug() << "Disconnecting all robots";
    
    while (!m_connections.isEmpty()) {
        disconnectRobot(0);
    }
}

void BleConnectionManager::sendToRobot(int index, const QByteArray &data)
{
    if (index < 0 || index >= m_connections.size()) {
        qWarning() << "Invalid robot index:" << index;
        return;
    }

    m_connections[index]->sendData(data);
}

void BleConnectionManager::sendToAll(const QByteArray &data)
{
    qDebug() << "Broadcasting data to all robots:" << data.toHex();
    
    for (BleRobotConnection *connection : m_connections) {
        connection->sendData(data);
    }
}

void BleConnectionManager::onConnectionStateChanged()
{
    BleRobotConnection *connection = qobject_cast<BleRobotConnection*>(sender());
    if (!connection) {
        return;
    }

    int index = findConnectionIndex(connection);
    if (index < 0) {
        return;
    }

    updateRobotModel(index);
    updateConnectedCount();

    if (connection->connectionState() == Robot::Ready) {
        emit robotConnected(index);
    }
}

void BleConnectionManager::onDataReceived(const QByteArray &data)
{
    BleRobotConnection *connection = qobject_cast<BleRobotConnection*>(sender());
    if (!connection) {
        return;
    }

    int index = findConnectionIndex(connection);
    if (index < 0) {
        return;
    }

    qDebug() << "Data received from robot" << index << ":" << data.toHex();

    // Update robot model with received data
    Robot robot = m_robotListModel->robotAt(index);
    robot.setLastPacketReceived(data);
    robot.setLastPacketTime(QDateTime::currentDateTime());
    m_robotListModel->updateRobot(index, robot);
}

void BleConnectionManager::onErrorOccurred(const QString &error)
{
    BleRobotConnection *connection = qobject_cast<BleRobotConnection*>(sender());
    if (!connection) {
        return;
    }

    int index = findConnectionIndex(connection);
    if (index < 0) {
        return;
    }

    qWarning() << "Robot" << index << "error:" << error;
    emit robotError(index, error);
}

int BleConnectionManager::findConnectionIndex(BleRobotConnection *connection)
{
    for (int i = 0; i < m_connections.size(); ++i) {
        if (m_connections[i] == connection) {
            return i;
        }
    }
    return -1;
}

int BleConnectionManager::findConnectionByAddress(const QString &address)
{
    for (int i = 0; i < m_connections.size(); ++i) {
        if (m_connections[i]->robotAddress() == address) {
            return i;
        }
    }
    return -1;
}

void BleConnectionManager::updateConnectedCount()
{
    int count = 0;
    for (BleRobotConnection *connection : m_connections) {
        if (connection->connectionState() == Robot::Ready || 
            connection->connectionState() == Robot::Connected) {
            count++;
        }
    }

    if (m_connectedCount != count) {
        m_connectedCount = count;
        emit connectedCountChanged();
    }
}

void BleConnectionManager::updateRobotModel(int index)
{
    if (index < 0 || index >= m_connections.size()) {
        return;
    }

    BleRobotConnection *connection = m_connections[index];
    Robot robot = m_robotListModel->robotAt(index);
    
    robot.setConnectionState(connection->connectionState());
    robot.setRssi(connection->rssi());
    robot.setName(connection->robotName());
    
    m_robotListModel->updateRobot(index, robot);
}
