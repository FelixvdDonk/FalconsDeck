#include "BleConnectionManager.h"
#include "BleDeviceScanner.h"
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
    int totalConnections = m_connections.size() + m_jbdConnections.size();
    if (totalConnections >= MAX_ROBOTS) {
        qWarning() << "Maximum number of robots reached:" << MAX_ROBOTS;
        return;
    }

    qDebug() << "Connecting to device:" << device.name() << address;

    // Check if this is a JBD BMS device (advertises service 0xFF00)
    bool isJbdDevice = false;
    const QList<QBluetoothUuid> serviceUuids = device.serviceUuids();
    for (const QBluetoothUuid &uuid : serviceUuids) {
        if (uuid == JbdBmsConnection::JBD_SERVICE_UUID) {
            isJbdDevice = true;
            break;
        }
    }

    // Add robot to model first
    Robot robot(m_nextRobotId++, device.name(), device.address());
    robot.setConnectionState(Robot::Connecting);
    robot.setRssi(device.rssi());
    m_robotListModel->addRobot(robot);

    int modelIndex = m_robotListModel->count() - 1;

    if (isJbdDevice) {
        qDebug() << "Detected JBD BMS device, using JbdBmsConnection";
        JbdBmsConnection *connection = new JbdBmsConnection(this);

        connect(connection, &JbdBmsConnection::connectionStateChanged,
                this, &BleConnectionManager::onJbdConnectionStateChanged);
        connect(connection, &JbdBmsConnection::bmsDataUpdated,
                this, &BleConnectionManager::onJbdBmsDataUpdated);
        connect(connection, &JbdBmsConnection::errorOccurred,
                this, &BleConnectionManager::onJbdErrorOccurred);

        m_jbdConnections.append(connection);
        connection->connectToDevice(device);
    } else {
        qDebug() << "Using NUS BleRobotConnection";
        BleRobotConnection *connection = new BleRobotConnection(this);

        connect(connection, &BleRobotConnection::connectionStateChanged,
                this, &BleConnectionManager::onConnectionStateChanged);
        connect(connection, &BleRobotConnection::dataReceived,
                this, &BleConnectionManager::onDataReceived);
        connect(connection, &BleRobotConnection::errorOccurred,
                this, &BleConnectionManager::onErrorOccurred);

        m_connections.append(connection);
        connection->connectToDevice(device);
    }
}

void BleConnectionManager::disconnectRobot(int index)
{
    if (index < 0 || index >= m_robotListModel->count()) {
        qWarning() << "Invalid robot index:" << index;
        return;
    }

    qDebug() << "Disconnecting robot at index:" << index;

    Robot robot = m_robotListModel->robotAt(index);
    QString address = robot.bluetoothAddress().toString();

    // Check if it's a NUS connection
    for (int i = 0; i < m_connections.size(); ++i) {
        if (m_connections[i]->robotAddress() == address) {
            BleRobotConnection *connection = m_connections[i];
            connection->disconnect();
            m_connections.removeAt(i);
            connection->deleteLater();
            break;
        }
    }

    // Check if it's a JBD connection
    for (int i = 0; i < m_jbdConnections.size(); ++i) {
        if (m_jbdConnections[i]->deviceAddress() == address) {
            JbdBmsConnection *connection = m_jbdConnections[i];
            connection->disconnect();
            m_jbdConnections.removeAt(i);
            connection->deleteLater();
            break;
        }
    }

    m_robotListModel->removeRobot(index);
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
    qDebug() << "Disconnecting all devices";
    
    while (m_robotListModel->count() > 0) {
        disconnectRobot(0);
    }
}

void BleConnectionManager::sendToRobot(int index, const QByteArray &data)
{
    if (index < 0 || index >= m_robotListModel->count()) {
        qWarning() << "Invalid robot index:" << index;
        return;
    }

    // Find NUS connection for this robot by address
    Robot robot = m_robotListModel->robotAt(index);
    QString address = robot.bluetoothAddress().toString();
    for (BleRobotConnection *connection : m_connections) {
        if (connection->robotAddress() == address) {
            connection->sendData(data);
            return;
        }
    }
    qWarning() << "No NUS connection found for robot at index:" << index << "(may be a BMS device)";
}

void BleConnectionManager::sendToRobot(int index, const QString &text)
{
    sendToRobot(index, text.toUtf8());
}

void BleConnectionManager::sendToAll(const QByteArray &data)
{
    qDebug() << "Broadcasting data to all robots:" << data.toHex();
    
    for (BleRobotConnection *connection : m_connections) {
        connection->sendData(data);
    }
}

void BleConnectionManager::sendToAll(const QString &text)
{
    sendToAll(text.toUtf8());
}

void BleConnectionManager::onConnectionStateChanged()
{
    BleRobotConnection *connection = qobject_cast<BleRobotConnection*>(sender());
    if (!connection) {
        return;
    }

    int index = findConnectionByAddress(connection->robotAddress());
    if (index < 0) {
        return;
    }

    Robot robot = m_robotListModel->robotAt(index);
    robot.setConnectionState(connection->connectionState());
    robot.setRssi(connection->rssi());
    robot.setName(connection->robotName());
    m_robotListModel->updateRobot(index, robot);

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

    int index = findConnectionByAddress(connection->robotAddress());
    if (index < 0) {
        return;
    }

    qDebug() << "Data received from robot" << index << ":" << data.toHex();

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

    int index = findConnectionByAddress(connection->robotAddress());
    if (index < 0) {
        return;
    }

    qWarning() << "Robot" << index << "error:" << error;
    emit robotError(index, error);
}

void BleConnectionManager::onJbdConnectionStateChanged()
{
    JbdBmsConnection *connection = qobject_cast<JbdBmsConnection*>(sender());
    if (!connection) {
        return;
    }

    int index = findConnectionByAddress(connection->deviceAddress());
    if (index < 0) {
        return;
    }

    Robot robot = m_robotListModel->robotAt(index);
    robot.setConnectionState(connection->connectionState());
    robot.setRssi(connection->rssi());
    robot.setName(connection->deviceName());
    m_robotListModel->updateRobot(index, robot);

    updateConnectedCount();

    if (connection->connectionState() == Robot::Ready) {
        emit robotConnected(index);
    }
}

void BleConnectionManager::onJbdBmsDataUpdated()
{
    JbdBmsConnection *connection = qobject_cast<JbdBmsConnection*>(sender());
    if (!connection) {
        return;
    }

    int index = findConnectionByAddress(connection->deviceAddress());
    if (index < 0) {
        return;
    }

    Robot robot = m_robotListModel->robotAt(index);
    robot.setTotalVoltage(connection->totalVoltage());
    robot.setCurrent(connection->current());
    robot.setSoc(connection->soc());
    robot.setLastPacketTime(QDateTime::currentDateTime());
    m_robotListModel->updateRobot(index, robot);
}

void BleConnectionManager::onJbdErrorOccurred(const QString &error)
{
    JbdBmsConnection *connection = qobject_cast<JbdBmsConnection*>(sender());
    if (!connection) {
        return;
    }

    int index = findConnectionByAddress(connection->deviceAddress());
    if (index < 0) {
        return;
    }

    qWarning() << "BMS" << index << "error:" << error;
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
    // Search in the robot model (covers both NUS and JBD connections)
    for (int i = 0; i < m_robotListModel->count(); ++i) {
        if (m_robotListModel->robotAt(i).bluetoothAddress().toString() == address) {
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
    for (JbdBmsConnection *connection : m_jbdConnections) {
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
    if (index < 0 || index >= m_robotListModel->count()) {
        return;
    }

    // This is only used for NUS connections now
    Robot robot = m_robotListModel->robotAt(index);
    QString address = robot.bluetoothAddress().toString();

    for (BleRobotConnection *connection : m_connections) {
        if (connection->robotAddress() == address) {
            robot.setConnectionState(connection->connectionState());
            robot.setRssi(connection->rssi());
            robot.setName(connection->robotName());
            m_robotListModel->updateRobot(index, robot);
            return;
        }
    }
}

void BleConnectionManager::updateJbdRobotModel(int index)
{
    // Not needed — handled directly in onJbdConnectionStateChanged / onJbdBmsDataUpdated
}

int BleConnectionManager::findJbdConnectionIndex(JbdBmsConnection *connection)
{
    for (int i = 0; i < m_jbdConnections.size(); ++i) {
        if (m_jbdConnections[i] == connection) {
            return i;
        }
    }
    return -1;
}
