#include "JbdBmsConnection.h"
#include <QDebug>

// JBD BMS (Xiaoxiang/SmartBMS) UUIDs
const QBluetoothUuid JbdBmsConnection::JBD_SERVICE_UUID     = QBluetoothUuid(static_cast<quint16>(0xFF00));
const QBluetoothUuid JbdBmsConnection::JBD_NOTIFY_CHAR_UUID = QBluetoothUuid(static_cast<quint16>(0xFF01));
const QBluetoothUuid JbdBmsConnection::JBD_WRITE_CHAR_UUID  = QBluetoothUuid(static_cast<quint16>(0xFF02));

JbdBmsConnection::JbdBmsConnection(QObject *parent)
    : QObject(parent)
    , m_controller(nullptr)
    , m_service(nullptr)
    , m_pollTimer(new QTimer(this))
    , m_connectionState(Robot::Disconnected)
    , m_rssi(-100)
    , m_serviceFound(false)
    , m_totalVoltage(0.0f)
    , m_current(0.0f)
    , m_soc(0)
{
    // Poll BMS data every 2 seconds
    m_pollTimer->setInterval(2000);
    connect(m_pollTimer, &QTimer::timeout, this, &JbdBmsConnection::requestBmsData);
}

JbdBmsConnection::~JbdBmsConnection()
{
    m_pollTimer->stop();
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

void JbdBmsConnection::connectToDevice(const QBluetoothDeviceInfo &device)
{
    if (m_controller) {
        qWarning() << "JbdBmsConnection: Already connected or connecting";
        return;
    }

    m_deviceName = device.name();
    m_deviceAddress = device.address();
    m_rssi = device.rssi();

    emit deviceNameChanged();
    emit deviceAddressChanged();
    emit rssiChanged();

    setConnectionState(Robot::Connecting);

    m_controller = QLowEnergyController::createCentral(device, this);

    // On Linux without CAP_NET_ADMIN, BlueZ can't auto-detect address types.
    // BLE peripherals typically use random addresses, so set it explicitly.
    m_controller->setRemoteAddressType(QLowEnergyController::RandomAddress);

    connect(m_controller, &QLowEnergyController::connected,
            this, &JbdBmsConnection::onControllerConnected);
    connect(m_controller, &QLowEnergyController::disconnected,
            this, &JbdBmsConnection::onControllerDisconnected);
    connect(m_controller, &QLowEnergyController::errorOccurred,
            this, &JbdBmsConnection::onControllerError);
    connect(m_controller, &QLowEnergyController::serviceDiscovered,
            this, &JbdBmsConnection::onServiceDiscovered);
    connect(m_controller, &QLowEnergyController::discoveryFinished,
            this, &JbdBmsConnection::onDiscoveryFinished);

    qDebug() << "JbdBmsConnection: Connecting to" << m_deviceName << m_deviceAddress.toString();
    m_controller->connectToDevice();
}

void JbdBmsConnection::disconnect()
{
    m_pollTimer->stop();
    if (m_controller) {
        m_controller->disconnectFromDevice();
    }
}

void JbdBmsConnection::onControllerConnected()
{
    qDebug() << "JbdBmsConnection: Controller connected, discovering services...";
    setConnectionState(Robot::Connected);
    m_serviceFound = false;
    m_controller->discoverServices();
}

void JbdBmsConnection::onControllerDisconnected()
{
    qDebug() << "JbdBmsConnection: Controller disconnected";
    m_pollTimer->stop();
    setConnectionState(Robot::Disconnected);

    if (m_service) {
        delete m_service;
        m_service = nullptr;
    }
}

void JbdBmsConnection::onControllerError(QLowEnergyController::Error error)
{
    QString errorString = m_controller->errorString();
    qWarning() << "JbdBmsConnection: Controller error:" << error << errorString;
    m_pollTimer->stop();
    setError(errorString);
    setConnectionState(Robot::Error);
}

void JbdBmsConnection::onServiceDiscovered(const QBluetoothUuid &serviceUuid)
{
    qDebug() << "JbdBmsConnection: Service discovered:" << serviceUuid.toString();
    if (serviceUuid == JBD_SERVICE_UUID) {
        qDebug() << "JbdBmsConnection: Found JBD BMS service (0xFF00)!";
        m_serviceFound = true;
    }
}

void JbdBmsConnection::onDiscoveryFinished()
{
    qDebug() << "JbdBmsConnection: Service discovery finished";

    if (!m_serviceFound) {
        setError("JBD BMS service (0xFF00) not found on device");
        setConnectionState(Robot::Error);
        return;
    }

    setupService();
}

void JbdBmsConnection::setupService()
{
    if (!m_controller) {
        return;
    }

    m_service = m_controller->createServiceObject(JBD_SERVICE_UUID, this);

    if (!m_service) {
        setError("Failed to create JBD BMS service object");
        setConnectionState(Robot::Error);
        return;
    }

    connect(m_service, &QLowEnergyService::stateChanged,
            this, &JbdBmsConnection::onServiceStateChanged);
    connect(m_service, &QLowEnergyService::characteristicChanged,
            this, &JbdBmsConnection::onCharacteristicChanged);

    qDebug() << "JbdBmsConnection: Discovering service details...";
    m_service->discoverDetails();
}

void JbdBmsConnection::onServiceStateChanged(QLowEnergyService::ServiceState state)
{
    qDebug() << "JbdBmsConnection: Service state changed:" << state;

    if (state == QLowEnergyService::RemoteServiceDiscovered) {
        // Find characteristics
        m_notifyCharacteristic = m_service->characteristic(JBD_NOTIFY_CHAR_UUID);
        m_writeCharacteristic  = m_service->characteristic(JBD_WRITE_CHAR_UUID);

        if (!m_notifyCharacteristic.isValid()) {
            setError("JBD notify characteristic (0xFF01) not found");
            setConnectionState(Robot::Error);
            return;
        }

        if (!m_writeCharacteristic.isValid()) {
            setError("JBD write characteristic (0xFF02) not found");
            setConnectionState(Robot::Error);
            return;
        }

        qDebug() << "JbdBmsConnection: Characteristics found, enabling notifications...";

        // Enable notifications on the notify characteristic
        QLowEnergyDescriptor notification = m_notifyCharacteristic.descriptor(
            QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);

        if (notification.isValid()) {
            m_service->writeDescriptor(notification, QByteArray::fromHex("0100"));
        } else {
            qWarning() << "JbdBmsConnection: CCCD descriptor not found, notifications may not work";
        }

        setConnectionState(Robot::Ready);
        qDebug() << "JbdBmsConnection: Connection ready, starting data polling";

        // Request initial data immediately, then start polling
        requestBmsData();
        m_pollTimer->start();
    }
}

void JbdBmsConnection::onCharacteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                                const QByteArray &value)
{
    if (characteristic.uuid() != JBD_NOTIFY_CHAR_UUID) {
        return;
    }

    // JBD BMS may send data in multiple BLE notifications that need assembling.
    // A frame starts with 0xDD and ends with 0x77.
    // Append to buffer and check for complete frame.

    m_frameBuffer.append(value);

    // Look for a complete frame: DD <cmd> <status> <len> <data...> <crc_hi> <crc_lo> 77
    while (m_frameBuffer.size() >= 7) {
        // Find start byte
        int startIdx = m_frameBuffer.indexOf(static_cast<char>(JBD_PKT_START));
        if (startIdx < 0) {
            m_frameBuffer.clear();
            return;
        }
        if (startIdx > 0) {
            // Discard bytes before start
            m_frameBuffer.remove(0, startIdx);
        }

        if (m_frameBuffer.size() < 4) {
            return; // Need more data
        }

        uint8_t dataLen = static_cast<uint8_t>(m_frameBuffer[3]);
        int frameLen = 4 + dataLen + 3; // header(4) + data + crc(2) + end(1)

        if (m_frameBuffer.size() < frameLen) {
            return; // Need more data
        }

        // Check end byte
        if (static_cast<uint8_t>(m_frameBuffer[frameLen - 1]) != JBD_PKT_END) {
            // Invalid frame, skip start byte and try again
            m_frameBuffer.remove(0, 1);
            continue;
        }

        // Extract complete frame
        QByteArray frame = m_frameBuffer.left(frameLen);
        m_frameBuffer.remove(0, frameLen);

        // Response format: DD <command> <status> <len> <data...> <crc_hi> <crc_lo> 77
        uint8_t command = static_cast<uint8_t>(frame[1]);
        uint8_t status  = static_cast<uint8_t>(frame[2]);

        qDebug() << "JbdBmsConnection: Frame received - cmd:" << Qt::hex << command
                 << "status:" << status << "len:" << dataLen
                 << "raw:" << frame.toHex(':');

        if (status != 0x00) {
            qWarning() << "JbdBmsConnection: BMS returned error status:" << Qt::hex << status;
            continue;
        }

        // Extract data portion (bytes 4 .. 4+dataLen-1)
        QByteArray payload = frame.mid(4, dataLen);

        switch (command) {
        case JBD_CMD_HWINFO:
            parseHardwareInfo(payload);
            break;
        case JBD_CMD_CELLINFO:
            qDebug() << "JbdBmsConnection: Cell info frame received (" << dataLen << "bytes)";
            break;
        default:
            qDebug() << "JbdBmsConnection: Unhandled command:" << Qt::hex << command;
            break;
        }
    }
}

void JbdBmsConnection::parseHardwareInfo(const QByteArray &data)
{
    // JBD hardware info frame (command 0x03) layout:
    //  Offset  Len  Description
    //   0       2   Total voltage       (uint16, * 0.01 = V)
    //   2       2   Current             (int16,  * 0.01 = A)
    //   4       2   Residual capacity   (uint16, * 0.01 = Ah)
    //   6       2   Nominal capacity    (uint16, * 0.01 = Ah)
    //   8       2   Cycle count
    //  10       2   Production date
    //  12       4   Balance status bitmask
    //  16       2   Protection status
    //  18       1   Software version
    //  19       1   State of charge (%)
    //  20       1   MOSFET status
    //  21       1   Cell count
    //  22       1   Temperature sensor count
    //  23+      2*N Temperature values

    if (data.size() < 23) {
        qWarning() << "JbdBmsConnection: Hardware info frame too short:" << data.size();
        return;
    }

    auto getUint16 = [&](int offset) -> uint16_t {
        return (static_cast<uint8_t>(data[offset]) << 8) |
                static_cast<uint8_t>(data[offset + 1]);
    };

    float newVoltage = getUint16(0) * 0.01f;
    float newCurrent = static_cast<int16_t>(getUint16(2)) * 0.01f;
    int   newSoc     = static_cast<uint8_t>(data[19]);

    bool changed = false;

    if (qFuzzyCompare(m_totalVoltage, newVoltage) == false) {
        m_totalVoltage = newVoltage;
        emit totalVoltageChanged();
        changed = true;
    }

    if (qFuzzyCompare(m_current, newCurrent) == false) {
        m_current = newCurrent;
        emit currentChanged();
        changed = true;
    }

    if (m_soc != newSoc) {
        m_soc = newSoc;
        emit socChanged();
        changed = true;
    }

    if (changed) {
        emit bmsDataUpdated();
    }

    qDebug() << "JbdBmsConnection:" << m_deviceName
             << "- Voltage:" << m_totalVoltage << "V"
             << "Current:" << m_current << "A"
             << "SoC:" << m_soc << "%";
}

void JbdBmsConnection::requestBmsData()
{
    if (m_connectionState != Robot::Ready) {
        return;
    }

    // Request hardware info (contains total voltage, current, SoC)
    sendCommand(JBD_CMD_HWINFO);
}

bool JbdBmsConnection::sendCommand(uint8_t command)
{
    if (!m_service || !m_writeCharacteristic.isValid()) {
        qWarning() << "JbdBmsConnection: Cannot send command, service not ready";
        return false;
    }

    // JBD read command frame: DD A5 <cmd> 00 <crc_hi> <crc_lo> 77
    // Checksum = 0x10000 - (cmd + data_len) & 0xFFFF
    uint8_t frame[7];
    frame[0] = JBD_PKT_START;   // 0xDD
    frame[1] = JBD_CMD_READ;    // 0xA5
    frame[2] = command;         // e.g. 0x03
    frame[3] = 0x00;            // data length = 0

    uint16_t crc = jbdChecksum(frame + 2, 2);
    frame[4] = (crc >> 8) & 0xFF;
    frame[5] = crc & 0xFF;
    frame[6] = JBD_PKT_END;     // 0x77

    QByteArray data(reinterpret_cast<const char*>(frame), sizeof(frame));
    qDebug() << "JbdBmsConnection: Sending command" << Qt::hex << command << "frame:" << data.toHex(':');

    m_service->writeCharacteristic(m_writeCharacteristic, data, QLowEnergyService::WriteWithoutResponse);
    return true;
}

uint16_t JbdBmsConnection::jbdChecksum(const uint8_t *data, uint16_t len)
{
    uint16_t checksum = 0x0000;
    for (uint16_t i = 0; i < len; i++) {
        checksum -= data[i];
    }
    return checksum;
}

void JbdBmsConnection::setConnectionState(Robot::ConnectionState state)
{
    if (m_connectionState != state) {
        m_connectionState = state;
        emit connectionStateChanged();
    }
}

void JbdBmsConnection::setError(const QString &error)
{
    m_lastError = error;
    emit errorOccurred(error);
}
