#include "FalconsRobotConnection.h"
#include <QDebug>
#include <QDataStream>

// Must match RobotBlePeripheral UUIDs exactly
const QBluetoothUuid FalconsRobotConnection::FALCONS_SERVICE_UUID =
    QBluetoothUuid(QStringLiteral("FA1C0001-B5A3-F393-E0A9-E50E24DCCA9E"));
const QBluetoothUuid FalconsRobotConnection::CHAR_PLAY_STATE_UUID =
    QBluetoothUuid(QStringLiteral("FA1C0002-B5A3-F393-E0A9-E50E24DCCA9E"));
const QBluetoothUuid FalconsRobotConnection::CHAR_WIFI_SSID_UUID =
    QBluetoothUuid(QStringLiteral("FA1C0003-B5A3-F393-E0A9-E50E24DCCA9E"));
const QBluetoothUuid FalconsRobotConnection::CHAR_WIFI_LIST_UUID =
    QBluetoothUuid(QStringLiteral("FA1C0004-B5A3-F393-E0A9-E50E24DCCA9E"));
const QBluetoothUuid FalconsRobotConnection::CHAR_BATTERY_VOLTAGE_UUID =
    QBluetoothUuid(QStringLiteral("FA1C0005-B5A3-F393-E0A9-E50E24DCCA9E"));
const QBluetoothUuid FalconsRobotConnection::CHAR_ROBOT_IDENTITY_UUID =
    QBluetoothUuid(QStringLiteral("FA1C0006-B5A3-F393-E0A9-E50E24DCCA9E"));

FalconsRobotConnection::FalconsRobotConnection(QObject *parent)
    : QObject(parent)
    , m_controller(nullptr)
    , m_service(nullptr)
    , m_connectionState(Robot::Disconnected)
    , m_rssi(-100)
    , m_serviceFound(false)
    , m_playState(0)
    , m_batteryVoltage(0.0f)
{
}

FalconsRobotConnection::~FalconsRobotConnection()
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

bool FalconsRobotConnection::isFalconsDevice(const QBluetoothDeviceInfo &device)
{
    const QList<QBluetoothUuid> serviceUuids = device.serviceUuids();
    for (const QBluetoothUuid &uuid : serviceUuids) {
        if (uuid == FALCONS_SERVICE_UUID) {
            return true;
        }
    }
    // Also match by name pattern
    return device.name().startsWith("Falcons-");
}

void FalconsRobotConnection::connectToDevice(const QBluetoothDeviceInfo &device)
{
    if (m_controller) {
        qWarning() << "FalconsRobotConnection: Already connected or connecting";
        return;
    }

    m_robotName = device.name();
    m_deviceAddress = device.address();
    m_rssi = device.rssi();

    emit robotNameChanged();
    emit robotAddressChanged();
    emit rssiChanged();

    setConnectionState(Robot::Connecting);

    m_controller = QLowEnergyController::createCentral(device, this);
    m_controller->setRemoteAddressType(QLowEnergyController::RandomAddress);

    connect(m_controller, &QLowEnergyController::connected,
            this, &FalconsRobotConnection::onControllerConnected);
    connect(m_controller, &QLowEnergyController::disconnected,
            this, &FalconsRobotConnection::onControllerDisconnected);
    connect(m_controller, &QLowEnergyController::errorOccurred,
            this, &FalconsRobotConnection::onControllerError);
    connect(m_controller, &QLowEnergyController::serviceDiscovered,
            this, &FalconsRobotConnection::onServiceDiscovered);
    connect(m_controller, &QLowEnergyController::discoveryFinished,
            this, &FalconsRobotConnection::onDiscoveryFinished);

    qDebug() << "FalconsRobotConnection: Connecting to" << m_robotName << m_deviceAddress.toString();
    m_controller->connectToDevice();
}

void FalconsRobotConnection::disconnect()
{
    if (m_controller) {
        m_controller->disconnectFromDevice();
    }
}

void FalconsRobotConnection::writePlayState(int state)
{
    if (!m_service || m_connectionState != Robot::Ready) {
        qWarning() << "FalconsRobotConnection: Cannot write play state, not ready";
        return;
    }

    if (!m_playStateChar.isValid()) {
        qWarning() << "FalconsRobotConnection: Play state characteristic not valid";
        return;
    }

    QByteArray data(1, static_cast<char>(state));
    m_service->writeCharacteristic(m_playStateChar, data);
    qDebug() << "FalconsRobotConnection: Writing play state:" << state;
}

void FalconsRobotConnection::writeWifiSsid(const QString &ssid)
{
    if (!m_service || m_connectionState != Robot::Ready) {
        qWarning() << "FalconsRobotConnection: Cannot write WiFi SSID, not ready";
        return;
    }

    if (!m_wifiSsidChar.isValid()) {
        qWarning() << "FalconsRobotConnection: WiFi SSID characteristic not valid";
        return;
    }

    QByteArray data = ssid.toUtf8();
    m_service->writeCharacteristic(m_wifiSsidChar, data);
    qDebug() << "FalconsRobotConnection: Writing WiFi SSID:" << ssid;
}

void FalconsRobotConnection::onControllerConnected()
{
    qDebug() << "FalconsRobotConnection: Controller connected, discovering services...";
    setConnectionState(Robot::Connected);
    m_serviceFound = false;
    m_controller->discoverServices();
}

void FalconsRobotConnection::onControllerDisconnected()
{
    qDebug() << "FalconsRobotConnection: Controller disconnected";
    setConnectionState(Robot::Disconnected);

    if (m_service) {
        delete m_service;
        m_service = nullptr;
    }
}

void FalconsRobotConnection::onControllerError(QLowEnergyController::Error error)
{
    QString errorString = m_controller->errorString();
    qWarning() << "FalconsRobotConnection: Controller error:" << error << errorString;
    setError(errorString);
    setConnectionState(Robot::Error);
}

void FalconsRobotConnection::onServiceDiscovered(const QBluetoothUuid &serviceUuid)
{
    qDebug() << "FalconsRobotConnection: Service discovered:" << serviceUuid.toString();
    if (serviceUuid == FALCONS_SERVICE_UUID) {
        qDebug() << "FalconsRobotConnection: Found Falcons Robot Control service!";
        m_serviceFound = true;
    }
}

void FalconsRobotConnection::onDiscoveryFinished()
{
    qDebug() << "FalconsRobotConnection: Service discovery finished";

    if (!m_serviceFound) {
        setError("Falcons Robot Control service not found");
        setConnectionState(Robot::Error);
        return;
    }

    setupService();
}

void FalconsRobotConnection::setupService()
{
    if (!m_controller) return;

    m_service = m_controller->createServiceObject(FALCONS_SERVICE_UUID, this);

    if (!m_service) {
        setError("Failed to create Falcons service object");
        setConnectionState(Robot::Error);
        return;
    }

    connect(m_service, &QLowEnergyService::stateChanged,
            this, &FalconsRobotConnection::onServiceStateChanged);
    connect(m_service, &QLowEnergyService::characteristicChanged,
            this, &FalconsRobotConnection::onCharacteristicChanged);
    connect(m_service, &QLowEnergyService::characteristicRead,
            this, &FalconsRobotConnection::onCharacteristicRead);

    qDebug() << "FalconsRobotConnection: Discovering service details...";
    m_service->discoverDetails();
}

void FalconsRobotConnection::enableNotifications(const QLowEnergyCharacteristic &characteristic)
{
    if (!characteristic.isValid()) return;

    QLowEnergyDescriptor cccd = characteristic.descriptor(
        QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);

    if (cccd.isValid()) {
        m_service->writeDescriptor(cccd, QByteArray::fromHex("0100"));
    }
}

void FalconsRobotConnection::onServiceStateChanged(QLowEnergyService::ServiceState state)
{
    qDebug() << "FalconsRobotConnection: Service state changed:" << state;

    if (state == QLowEnergyService::RemoteServiceDiscovered) {
        // Find all characteristics
        m_playStateChar      = m_service->characteristic(CHAR_PLAY_STATE_UUID);
        m_wifiSsidChar       = m_service->characteristic(CHAR_WIFI_SSID_UUID);
        m_wifiListChar       = m_service->characteristic(CHAR_WIFI_LIST_UUID);
        m_batteryVoltageChar = m_service->characteristic(CHAR_BATTERY_VOLTAGE_UUID);
        m_robotIdentityChar  = m_service->characteristic(CHAR_ROBOT_IDENTITY_UUID);

        // Enable notifications on all characteristics
        enableNotifications(m_playStateChar);
        enableNotifications(m_wifiSsidChar);
        enableNotifications(m_wifiListChar);
        enableNotifications(m_batteryVoltageChar);
        enableNotifications(m_robotIdentityChar);

        // Read initial values
        readAllCharacteristics();

        setConnectionState(Robot::Ready);
        qDebug() << "FalconsRobotConnection: Connection ready!";
    }
}

void FalconsRobotConnection::readAllCharacteristics()
{
    if (!m_service) return;

    if (m_playStateChar.isValid())
        m_service->readCharacteristic(m_playStateChar);
    if (m_wifiSsidChar.isValid())
        m_service->readCharacteristic(m_wifiSsidChar);
    if (m_wifiListChar.isValid())
        m_service->readCharacteristic(m_wifiListChar);
    if (m_batteryVoltageChar.isValid())
        m_service->readCharacteristic(m_batteryVoltageChar);
    if (m_robotIdentityChar.isValid())
        m_service->readCharacteristic(m_robotIdentityChar);
}

void FalconsRobotConnection::onCharacteristicChanged(
    const QLowEnergyCharacteristic &characteristic,
    const QByteArray &value)
{
    QBluetoothUuid uuid = characteristic.uuid();

    if (uuid == CHAR_PLAY_STATE_UUID)           parsePlayState(value);
    else if (uuid == CHAR_WIFI_SSID_UUID)       parseWifiSsid(value);
    else if (uuid == CHAR_WIFI_LIST_UUID)        parseWifiList(value);
    else if (uuid == CHAR_BATTERY_VOLTAGE_UUID)  parseBatteryVoltage(value);
    else if (uuid == CHAR_ROBOT_IDENTITY_UUID)   parseRobotIdentity(value);
}

void FalconsRobotConnection::onCharacteristicRead(
    const QLowEnergyCharacteristic &characteristic,
    const QByteArray &value)
{
    // Same handling as notification
    onCharacteristicChanged(characteristic, value);
}

void FalconsRobotConnection::parsePlayState(const QByteArray &value)
{
    if (value.isEmpty()) return;

    int newState = static_cast<uint8_t>(value[0]);
    if (newState != m_playState) {
        m_playState = newState;
        emit playStateChanged();
        emit robotDataUpdated();
        qDebug() << "FalconsRobotConnection:" << m_robotName << "play state:" << m_playState;
    }
}

void FalconsRobotConnection::parseWifiSsid(const QByteArray &value)
{
    QString newSsid = QString::fromUtf8(value);
    if (newSsid != m_wifiSsid) {
        m_wifiSsid = newSsid;
        emit wifiSsidChanged();
        emit robotDataUpdated();
        qDebug() << "FalconsRobotConnection:" << m_robotName << "WiFi SSID:" << m_wifiSsid;
    }
}

void FalconsRobotConnection::parseWifiList(const QByteArray &value)
{
    QString listStr = QString::fromUtf8(value);
    QStringList newList = listStr.split('\n', Qt::SkipEmptyParts);
    if (newList != m_wifiList) {
        m_wifiList = newList;
        emit wifiListChanged();
        emit robotDataUpdated();
        qDebug() << "FalconsRobotConnection:" << m_robotName << "WiFi list:" << m_wifiList.size() << "networks";
    }
}

void FalconsRobotConnection::parseBatteryVoltage(const QByteArray &value)
{
    if (value.size() < 4) return;

    QDataStream stream(value);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(QDataStream::LittleEndian);

    float newVoltage;
    stream >> newVoltage;

    if (!qFuzzyCompare(newVoltage, m_batteryVoltage)) {
        m_batteryVoltage = newVoltage;
        emit batteryVoltageChanged();
        emit robotDataUpdated();
    }
}

void FalconsRobotConnection::parseRobotIdentity(const QByteArray &value)
{
    QString newIdentity = QString::fromUtf8(value);
    if (newIdentity != m_robotIdentity) {
        m_robotIdentity = newIdentity;
        emit robotIdentityChanged();
        emit robotDataUpdated();

        // Use robot identity as display name if available
        if (!newIdentity.isEmpty()) {
            m_robotName = "Falcons-" + newIdentity;
            emit robotNameChanged();
        }

        qDebug() << "FalconsRobotConnection:" << m_robotName << "identity:" << m_robotIdentity;
    }
}

void FalconsRobotConnection::setConnectionState(Robot::ConnectionState state)
{
    if (m_connectionState != state) {
        m_connectionState = state;
        emit connectionStateChanged();
    }
}

void FalconsRobotConnection::setError(const QString &error)
{
    m_lastError = error;
    emit errorOccurred(error);
}
