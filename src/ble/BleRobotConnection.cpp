#include "BleRobotConnection.h"
#include <QDebug>

// Nordic UART Service UUID definitions
const QBluetoothUuid BleRobotConnection::NUS_SERVICE_UUID = QBluetoothUuid(QStringLiteral("6E400001-B5A3-F393-E0A9-E50E24DCCA9E"));
const QBluetoothUuid BleRobotConnection::NUS_RX_CHAR_UUID = QBluetoothUuid(QStringLiteral("6E400002-B5A3-F393-E0A9-E50E24DCCA9E"));
const QBluetoothUuid BleRobotConnection::NUS_TX_CHAR_UUID = QBluetoothUuid(QStringLiteral("6E400003-B5A3-F393-E0A9-E50E24DCCA9E"));

BleRobotConnection::BleRobotConnection(QObject *parent)
    : QObject(parent)
    , m_controller(nullptr)
    , m_service(nullptr)
    , m_connectionState(Robot::Disconnected)
    , m_rssi(-100)
    , m_serviceFound(false)
{
}

BleRobotConnection::~BleRobotConnection()
{
    if (m_service) {
        delete m_service;
        m_service = nullptr;
    }
    if (m_controller) {
        m_controller->disconnectFromDevice();
        delete m_controller;
        m_controller = nullptr;
    }
}

void BleRobotConnection::connectToDevice(const QBluetoothDeviceInfo &device)
{
    if (m_controller) {
        qWarning() << "Already connected or connecting";
        return;
    }

    m_robotName = device.name();
    m_deviceAddress = device.address();
    m_rssi = device.rssi();

    emit robotNameChanged();
    emit robotAddressChanged();
    emit rssiChanged();

    setConnectionState(Robot::Connecting);

    // Create controller
    m_controller = QLowEnergyController::createCentral(device, this);

    connect(m_controller, &QLowEnergyController::connected,
            this, &BleRobotConnection::onControllerConnected);
    connect(m_controller, &QLowEnergyController::disconnected,
            this, &BleRobotConnection::onControllerDisconnected);
    connect(m_controller, &QLowEnergyController::errorOccurred,
            this, &BleRobotConnection::onControllerError);
    connect(m_controller, &QLowEnergyController::serviceDiscovered,
            this, &BleRobotConnection::onServiceDiscovered);
    connect(m_controller, &QLowEnergyController::discoveryFinished,
            this, &BleRobotConnection::onDiscoveryFinished);

    qDebug() << "Connecting to device:" << m_robotName << m_deviceAddress.toString();
    m_controller->connectToDevice();
}

void BleRobotConnection::disconnect()
{
    if (m_controller) {
        m_controller->disconnectFromDevice();
    }
}

void BleRobotConnection::sendData(const QByteArray &data)
{
    if (!m_service || m_connectionState != Robot::Ready) {
        qWarning() << "Cannot send data: not ready";
        return;
    }

    if (!m_rxCharacteristic.isValid()) {
        qWarning() << "RX characteristic not valid";
        return;
    }

    // Check if we need to chunk the data (typical BLE MTU is 20-23 bytes)
    const int maxChunkSize = 20;
    if (data.size() <= maxChunkSize) {
        m_service->writeCharacteristic(m_rxCharacteristic, data, QLowEnergyService::WriteWithoutResponse);
    } else {
        // Send in chunks
        for (int i = 0; i < data.size(); i += maxChunkSize) {
            QByteArray chunk = data.mid(i, maxChunkSize);
            m_service->writeCharacteristic(m_rxCharacteristic, chunk, QLowEnergyService::WriteWithoutResponse);
        }
    }
}

void BleRobotConnection::onControllerConnected()
{
    qDebug() << "Controller connected, discovering services...";
    setConnectionState(Robot::Connected);
    m_serviceFound = false;
    m_controller->discoverServices();
}

void BleRobotConnection::onControllerDisconnected()
{
    qDebug() << "Controller disconnected";
    setConnectionState(Robot::Disconnected);
    
    if (m_service) {
        delete m_service;
        m_service = nullptr;
    }
}

void BleRobotConnection::onControllerError(QLowEnergyController::Error error)
{
    QString errorString = m_controller->errorString();
    qWarning() << "Controller error:" << error << errorString;
    setError(errorString);
    setConnectionState(Robot::Error);
}

void BleRobotConnection::onServiceDiscovered(const QBluetoothUuid &serviceUuid)
{
    qDebug() << "Service discovered:" << serviceUuid.toString();
    if (serviceUuid == NUS_SERVICE_UUID) {
        qDebug() << "Found Nordic UART Service!";
        m_serviceFound = true;
    }
}

void BleRobotConnection::onDiscoveryFinished()
{
    qDebug() << "Service discovery finished";
    
    if (!m_serviceFound) {
        setError("Nordic UART Service not found");
        setConnectionState(Robot::Error);
        return;
    }

    setupService();
}

void BleRobotConnection::setupService()
{
    if (!m_controller) {
        return;
    }

    m_service = m_controller->createServiceObject(NUS_SERVICE_UUID, this);
    
    if (!m_service) {
        setError("Failed to create service object");
        setConnectionState(Robot::Error);
        return;
    }

    connect(m_service, &QLowEnergyService::stateChanged,
            this, &BleRobotConnection::onServiceStateChanged);
    connect(m_service, &QLowEnergyService::characteristicChanged,
            this, &BleRobotConnection::onCharacteristicChanged);
    connect(m_service, &QLowEnergyService::characteristicWritten,
            this, &BleRobotConnection::onCharacteristicWritten);

    qDebug() << "Discovering service details...";
    m_service->discoverDetails();
}

void BleRobotConnection::onServiceStateChanged(QLowEnergyService::ServiceState state)
{
    qDebug() << "Service state changed:" << state;

    if (state == QLowEnergyService::RemoteServiceDiscovered) {
        // Find characteristics
        m_rxCharacteristic = m_service->characteristic(NUS_RX_CHAR_UUID);
        m_txCharacteristic = m_service->characteristic(NUS_TX_CHAR_UUID);

        if (!m_rxCharacteristic.isValid()) {
            setError("RX characteristic not found");
            setConnectionState(Robot::Error);
            return;
        }

        if (!m_txCharacteristic.isValid()) {
            setError("TX characteristic not found");
            setConnectionState(Robot::Error);
            return;
        }

        qDebug() << "Characteristics found, enabling notifications...";

        // Enable notifications on TX characteristic
        QLowEnergyDescriptor notification = m_txCharacteristic.descriptor(
            QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);

        if (notification.isValid()) {
            m_service->writeDescriptor(notification, QByteArray::fromHex("0100"));
        }

        setConnectionState(Robot::Ready);
        qDebug() << "Connection ready!";
    }
}

void BleRobotConnection::onCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value)
{
    if (characteristic.uuid() == NUS_TX_CHAR_UUID) {
        emit dataReceived(value);
    }
}

void BleRobotConnection::onCharacteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &value)
{
    Q_UNUSED(characteristic)
    Q_UNUSED(value)
    // Data was successfully written
}

void BleRobotConnection::setConnectionState(Robot::ConnectionState state)
{
    if (m_connectionState != state) {
        m_connectionState = state;
        emit connectionStateChanged();
    }
}

void BleRobotConnection::setError(const QString &error)
{
    m_lastError = error;
    emit errorOccurred(error);
}
