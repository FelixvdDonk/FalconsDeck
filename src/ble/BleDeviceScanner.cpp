#include "BleDeviceScanner.h"
#include <QDebug>
#include <QVariantMap>

BleDeviceScanner::BleDeviceScanner(QObject *parent)
    : QObject(parent)
    , m_scanning(false)
{
    m_discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    m_discoveryAgent->setLowEnergyDiscoveryTimeout(5000);

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
        qDebug() << "Already scanning";
        return;
    }

    qDebug() << "Starting BLE scan...";
    m_discoveredDevices.clear();
    m_deviceInfoList.clear();
    emit discoveredDevicesChanged();

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

void BleDeviceScanner::onDeviceDiscovered(const QBluetoothDeviceInfo &device)
{
    // Only process Low Energy devices
    if (!(device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration)) {
        return;
    }

    qDebug() << "Device discovered:" << device.name() << device.address().toString() << "RSSI:" << device.rssi();
    
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
