#ifndef BLEDEVICESCANNER_H
#define BLEDEVICESCANNER_H

#include <QObject>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QBluetoothUuid>
#include <QVariantList>

class BleDeviceScanner : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool scanning READ isScanning NOTIFY scanningChanged)
    Q_PROPERTY(QVariantList discoveredDevices READ discoveredDevices NOTIFY discoveredDevicesChanged)
    Q_PROPERTY(bool filterEnabled READ isFilterEnabled WRITE setFilterEnabled NOTIFY filterEnabledChanged)

public:
    // JBD (Xiaoxiang/SmartBMS) BLE service UUID (0xFF00)
    static const QBluetoothUuid JBD_BMS_SERVICE_UUID;

    explicit BleDeviceScanner(QObject *parent = nullptr);
    ~BleDeviceScanner();

    bool isScanning() const { return m_scanning; }
    QVariantList discoveredDevices() const { return m_discoveredDevices; }
    bool isFilterEnabled() const { return m_filterEnabled; }

public slots:
    void startScan();
    void stopScan();
    void setFilterEnabled(bool enabled);

    Q_INVOKABLE QBluetoothDeviceInfo getDeviceAt(int index) const;
    Q_INVOKABLE QBluetoothDeviceInfo getDeviceByAddress(const QString &address) const;

signals:
    void scanningChanged();
    void discoveredDevicesChanged();
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void scanFinished();
    void scanError(const QString &error);
    void filterEnabledChanged();

private slots:
    void onDeviceDiscovered(const QBluetoothDeviceInfo &device);
    void onScanFinished();
    void onScanError(QBluetoothDeviceDiscoveryAgent::Error error);

private:
    void updateDeviceList(const QBluetoothDeviceInfo &device);
    bool isJbdBmsDevice(const QBluetoothDeviceInfo &device) const;

    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent;
    QVariantList m_discoveredDevices;
    QList<QBluetoothDeviceInfo> m_deviceInfoList;
    bool m_scanning;
    bool m_filterEnabled;
};

#endif // BLEDEVICESCANNER_H
