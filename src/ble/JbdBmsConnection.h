#ifndef JDBBMSCONNECTION_H
#define JDBBMSCONNECTION_H

#include <QObject>
#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QBluetoothUuid>
#include <QTimer>
#include "src/models/Robot.h"

class JbdBmsConnection : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Robot::ConnectionState connectionState READ connectionState NOTIFY connectionStateChanged)
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY deviceNameChanged)
    Q_PROPERTY(QString deviceAddress READ deviceAddress NOTIFY deviceAddressChanged)
    Q_PROPERTY(int rssi READ rssi NOTIFY rssiChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY errorOccurred)
    Q_PROPERTY(float totalVoltage READ totalVoltage NOTIFY totalVoltageChanged)
    Q_PROPERTY(float current READ current NOTIFY currentChanged)
    Q_PROPERTY(int soc READ soc NOTIFY socChanged)

public:
    // JBD BMS BLE UUIDs
    static const QBluetoothUuid JBD_SERVICE_UUID;        // 0xFF00
    static const QBluetoothUuid JBD_NOTIFY_CHAR_UUID;    // 0xFF01  - Notify/Read
    static const QBluetoothUuid JBD_WRITE_CHAR_UUID;     // 0xFF02  - Write

    // JBD protocol constants
    static const uint8_t JBD_PKT_START = 0xDD;
    static const uint8_t JBD_PKT_END   = 0x77;
    static const uint8_t JBD_CMD_READ  = 0xA5;
    static const uint8_t JBD_CMD_HWINFO   = 0x03;
    static const uint8_t JBD_CMD_CELLINFO = 0x04;

    explicit JbdBmsConnection(QObject *parent = nullptr);
    ~JbdBmsConnection();

    Robot::ConnectionState connectionState() const { return m_connectionState; }
    QString deviceName() const { return m_deviceName; }
    QString deviceAddress() const { return m_deviceAddress.toString(); }
    int rssi() const { return m_rssi; }
    QString lastError() const { return m_lastError; }
    float totalVoltage() const { return m_totalVoltage; }
    float current() const { return m_current; }
    int soc() const { return m_soc; }

    QBluetoothAddress bleAddress() const { return m_deviceAddress; }

public slots:
    void connectToDevice(const QBluetoothDeviceInfo &device);
    void disconnect();

signals:
    void connectionStateChanged();
    void deviceNameChanged();
    void deviceAddressChanged();
    void rssiChanged();
    void errorOccurred(const QString &error);
    void totalVoltageChanged();
    void currentChanged();
    void socChanged();
    void bmsDataUpdated();

private slots:
    void onControllerConnected();
    void onControllerDisconnected();
    void onControllerError(QLowEnergyController::Error error);
    void onServiceDiscovered(const QBluetoothUuid &serviceUuid);
    void onDiscoveryFinished();
    void onServiceStateChanged(QLowEnergyService::ServiceState state);
    void onCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    void requestBmsData();

private:
    void setConnectionState(Robot::ConnectionState state);
    void setError(const QString &error);
    void setupService();
    bool sendCommand(uint8_t command);
    void parseHardwareInfo(const QByteArray &data);
    static uint16_t jbdChecksum(const uint8_t *data, uint16_t len);

    QLowEnergyController *m_controller;
    QLowEnergyService *m_service;
    QLowEnergyCharacteristic m_notifyCharacteristic;
    QLowEnergyCharacteristic m_writeCharacteristic;
    QTimer *m_pollTimer;
    QByteArray m_frameBuffer;

    Robot::ConnectionState m_connectionState;
    QString m_deviceName;
    QBluetoothAddress m_deviceAddress;
    int m_rssi;
    QString m_lastError;
    bool m_serviceFound;

    // BMS data
    float m_totalVoltage;
    float m_current;
    int m_soc;
};

#endif // JDBBMSCONNECTION_H
