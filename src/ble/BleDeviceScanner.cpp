#include "BleDeviceScanner.h"
#include <QDebug>
#include <QVariantMap>

// JBD BMS (Xiaoxiang/SmartBMS) uses the 0xFF00 BLE service with characteristics 0xFF01 (notify) and 0xFF02 (write)
const QBluetoothUuid BleDeviceScanner::JBD_BMS_SERVICE_UUID = QBluetoothUuid(static_cast<quint16>(0xFF00));

// Falcons Robot Control service UUID
const QBluetoothUuid BleDeviceScanner::FALCONS_SERVICE_UUID = QBluetoothUuid(QStringLiteral("FA1C0001-B5A3-F393-E0A9-E50E24DCCA9E"));

BleDeviceScanner::BleDeviceScanner(QObject *parent)
    : QObject(parent)
    , m_scanning(false)
    , m_filterEnabled(true)
{
    m_discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    m_discoveryAgent->setLowEnergyDiscoveryTimeout(0);

    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &BleDeviceScanner::onDeviceDiscovered);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, &BleDeviceScanner::onScanFinished);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred,
            this, &BleDeviceScanner::onScanError);
}

BleDeviceScanner::~BleDeviceScanner()
{
    if (m_discoveryAgent->isActive()) {
        m_discoveryAgent->stop();
    }
}

void BleDeviceScanner::startScan()
{
    if (m_scanning) {
        return;
    }

    qDebug() << "Starting BLE scan...";

    m_scanning = true;
    emit scanningChanged();

    m_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

void BleDeviceScanner::stopScan()
{
    if (!m_scanning) {
        return;
    }

    qDebug() << "Stopping BLE scan...";
    m_discoveryAgent->stop();
    
    m_scanning = false;
    emit scanningChanged();
}

void BleDeviceScanner::setFilterEnabled(bool enabled)
{
    if (m_filterEnabled == enabled)
        return;

    m_filterEnabled = enabled;
    emit filterEnabledChanged();
    qDebug() << "JBD BMS filter" << (enabled ? "enabled" : "disabled");
}

bool BleDeviceScanner::isJbdBmsDevice(const QBluetoothDeviceInfo &device) const
{
    // Check if the device advertises the JBD BMS service UUID (0xFF00)
    const QList<QBluetoothUuid> serviceUuids = device.serviceUuids();
    for (const QBluetoothUuid &uuid : serviceUuids) {
        if (uuid == JBD_BMS_SERVICE_UUID) {
            return true;
        }
    }

    return false;
}

bool BleDeviceScanner::isFalconsDevice(const QBluetoothDeviceInfo &device) const
{
    // Check if the device advertises the Falcons Robot Control service UUID
    const QList<QBluetoothUuid> serviceUuids = device.serviceUuids();
    for (const QBluetoothUuid &uuid : serviceUuids) {
        if (uuid == FALCONS_SERVICE_UUID) {
            return true;
        }
    }
    // Also match by name pattern
    return device.name().startsWith("Falcons-");
}

void BleDeviceScanner::onDeviceDiscovered(const QBluetoothDeviceInfo &device)
{
    // Only process Low Energy devices
    if (!(device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration)) {
        return;
    }

    // Filter for JBD BMS devices if enabled (but always pass Falcons robots)
    if (m_filterEnabled && !isJbdBmsDevice(device) && !isFalconsDevice(device)) {
        return;
    }

    qDebug() << "Device discovered:" << device.name() << device.address().toString()
             << "RSSI:" << device.rssi();

    updateDeviceList(device);
    emit deviceDiscovered(device);
}

void BleDeviceScanner::onScanFinished()
{
    qDebug() << "BLE scan finished. Found" << m_discoveredDevices.count() << "devices";
    
    m_scanning = false;
    emit scanningChanged();
    emit scanFinished();
}

void BleDeviceScanner::onScanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    QString errorString;
    switch (error) {
    case QBluetoothDeviceDiscoveryAgent::PoweredOffError:
        errorString = "Bluetooth adapter is powered off";
        break;
    case QBluetoothDeviceDiscoveryAgent::InputOutputError:
        errorString = "I/O error occurred during discovery";
        break;
    case QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError:
        errorString = "Invalid Bluetooth adapter";
        break;
    case QBluetoothDeviceDiscoveryAgent::UnsupportedPlatformError:
        errorString = "Device discovery is not supported on this platform";
        break;
    case QBluetoothDeviceDiscoveryAgent::UnsupportedDiscoveryMethod:
        errorString = "Discovery method not supported";
        break;
    default:
        errorString = "Unknown error occurred";
        break;
    }

    qWarning() << "Scan error:" << errorString;
    
    m_scanning = false;
    emit scanningChanged();
    emit scanError(errorString);
}

QBluetoothDeviceInfo BleDeviceScanner::getDeviceAt(int index) const
{
    if (index >= 0 && index < m_deviceInfoList.size()) {
        return m_deviceInfoList.at(index);
    }
    return QBluetoothDeviceInfo();
}

QBluetoothDeviceInfo BleDeviceScanner::getDeviceByAddress(const QString &address) const
{
    for (const QBluetoothDeviceInfo &device : m_deviceInfoList) {
        if (device.address().toString() == address) {
            return device;
        }
    }
    return QBluetoothDeviceInfo();
}

void BleDeviceScanner::updateDeviceList(const QBluetoothDeviceInfo &device)
{
    // Check if device already exists (by address)
    bool found = false;
    for (int i = 0; i < m_deviceInfoList.size(); ++i) {
        if (m_deviceInfoList[i].address() == device.address()) {
            // Update existing device (RSSI may have changed)
            m_deviceInfoList[i] = device;
            
            QVariantMap deviceMap;
            deviceMap["name"] = device.name().isEmpty() ? "Unknown Device" : device.name();
            deviceMap["address"] = device.address().toString();
            deviceMap["rssi"] = device.rssi();
            m_discoveredDevices[i] = deviceMap;
            
            found = true;
            break;
        }
    }

    if (!found) {
        // Add new device
        m_deviceInfoList.append(device);
        
        QVariantMap deviceMap;
        deviceMap["name"] = device.name().isEmpty() ? "Unknown Device" : device.name();
        deviceMap["address"] = device.address().toString();
        deviceMap["rssi"] = device.rssi();
        m_discoveredDevices.append(deviceMap);
    }

    emit discoveredDevicesChanged();
}
