#ifndef BLEROBOTCONNECTION_H
#define BLEROBOTCONNECTION_H

#include <QObject>
#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QBluetoothUuid>
#include "src/models/Robot.h"

class BleRobotConnection : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Robot::ConnectionState connectionState READ connectionState NOTIFY connectionStateChanged)
    Q_PROPERTY(QString robotName READ robotName NOTIFY robotNameChanged)
    Q_PROPERTY(QString robotAddress READ robotAddress NOTIFY robotAddressChanged)
    Q_PROPERTY(int rssi READ rssi NOTIFY rssiChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY errorOccurred)

public:
    // Nordic UART Service UUIDs
    static const QBluetoothUuid NUS_SERVICE_UUID;
    static const QBluetoothUuid NUS_RX_CHAR_UUID;  // Write to robot
    static const QBluetoothUuid NUS_TX_CHAR_UUID;  // Receive from robot

    explicit BleRobotConnection(QObject *parent = nullptr);
    ~BleRobotConnection();

    Robot::ConnectionState connectionState() const { return m_connectionState; }
    QString robotName() const { return m_robotName; }
    QString robotAddress() const { return m_deviceAddress.toString(); }
    int rssi() const { return m_rssi; }
    QString lastError() const { return m_lastError; }

    QBluetoothAddress deviceAddress() const { return m_deviceAddress; }

public slots:
    void connectToDevice(const QBluetoothDeviceInfo &device);
    void disconnect();
    void sendData(const QByteArray &data);

signals:
    void dataReceived(const QByteArray &data);
    void connectionStateChanged();
    void robotNameChanged();
    void robotAddressChanged();
    void rssiChanged();
    void errorOccurred(const QString &error);

private slots:
    void onControllerConnected();
    void onControllerDisconnected();
    void onControllerError(QLowEnergyController::Error error);
    void onServiceDiscovered(const QBluetoothUuid &serviceUuid);
    void onDiscoveryFinished();
    void onServiceStateChanged(QLowEnergyService::ServiceState state);
    void onCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    void onCharacteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);

private:
    void setConnectionState(Robot::ConnectionState state);
    void setError(const QString &error);
    void setupService();

    QLowEnergyController *m_controller;
    QLowEnergyService *m_service;
    QLowEnergyCharacteristic m_rxCharacteristic;
    QLowEnergyCharacteristic m_txCharacteristic;

    Robot::ConnectionState m_connectionState;
    QString m_robotName;
    QBluetoothAddress m_deviceAddress;
    int m_rssi;
    QString m_lastError;
    bool m_serviceFound;
};

#endif // BLEROBOTCONNECTION_H
