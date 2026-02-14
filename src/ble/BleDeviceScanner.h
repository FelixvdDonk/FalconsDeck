#ifndef BLEDEVICESCANNER_H
#define BLEDEVICESCANNER_H

#include <QObject>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QVariantList>

class BleDeviceScanner : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool scanning READ isScanning NOTIFY scanningChanged)
    Q_PROPERTY(QVariantList discoveredDevices READ discoveredDevices NOTIFY discoveredDevicesChanged)

public:
    explicit BleDeviceScanner(QObject *parent = nullptr);
    ~BleDeviceScanner();

    bool isScanning() const { return m_scanning; }
    QVariantList discoveredDevices() const { return m_discoveredDevices; }

public slots:
    void startScan();
    void stopScan();

signals:
    void scanningChanged();
    void discoveredDevicesChanged();
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void scanFinished();
    void scanError(const QString &error);

private slots:
    void onDeviceDiscovered(const QBluetoothDeviceInfo &device);
    void onScanFinished();
    void onScanError(QBluetoothDeviceDiscoveryAgent::Error error);

private:
    void updateDeviceList(const QBluetoothDeviceInfo &device);

    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent;
    QVariantList m_discoveredDevices;
    QList<QBluetoothDeviceInfo> m_deviceInfoList;
    bool m_scanning;
};

#endif // BLEDEVICESCANNER_H
