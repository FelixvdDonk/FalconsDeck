#ifndef FALCONSROBOTCONNECTION_H
#define FALCONSROBOTCONNECTION_H

#include <QObject>
#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QBluetoothUuid>
#include <QTimer>
#include "src/models/Robot.h"

/**
 * FalconsRobotConnection - BLE central connection to a Falcons football robot.
 *
 * Connects to the custom Falcons Robot Control GATT service running on the robot's
 * RobotBlePeripheral, and provides read/write/notify access to:
 *   - Play state (read/write/notify)
 *   - WiFi SSID (read/write/notify)
 *   - WiFi list (read/notify)
 *   - Battery voltage (read/notify)
 *   - Robot identity (read/notify)
 */
class FalconsRobotConnection : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Robot::ConnectionState connectionState READ connectionState NOTIFY connectionStateChanged)
    Q_PROPERTY(QString robotName READ robotName NOTIFY robotNameChanged)
    Q_PROPERTY(QString robotAddress READ robotAddress NOTIFY robotAddressChanged)
    Q_PROPERTY(int rssi READ rssi NOTIFY rssiChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY errorOccurred)
    Q_PROPERTY(int playState READ playState NOTIFY playStateChanged)
    Q_PROPERTY(QString wifiSsid READ wifiSsid NOTIFY wifiSsidChanged)
    Q_PROPERTY(QStringList wifiList READ wifiList NOTIFY wifiListChanged)
    Q_PROPERTY(float batteryVoltage READ batteryVoltage NOTIFY batteryVoltageChanged)
    Q_PROPERTY(QString robotIdentity READ robotIdentity NOTIFY robotIdentityChanged)

public:
    // Falcons Robot Control Service UUIDs — must match RobotBlePeripheral
    static const QBluetoothUuid FALCONS_SERVICE_UUID;
    static const QBluetoothUuid CHAR_PLAY_STATE_UUID;
    static const QBluetoothUuid CHAR_WIFI_SSID_UUID;
    static const QBluetoothUuid CHAR_WIFI_LIST_UUID;
    static const QBluetoothUuid CHAR_BATTERY_VOLTAGE_UUID;
    static const QBluetoothUuid CHAR_ROBOT_IDENTITY_UUID;

    explicit FalconsRobotConnection(QObject *parent = nullptr);
    ~FalconsRobotConnection();

    Robot::ConnectionState connectionState() const { return m_connectionState; }
    QString robotName() const { return m_robotName; }
    QString robotAddress() const { return m_deviceAddress.toString(); }
    int rssi() const { return m_rssi; }
    QString lastError() const { return m_lastError; }
    int playState() const { return m_playState; }
    QString wifiSsid() const { return m_wifiSsid; }
    QStringList wifiList() const { return m_wifiList; }
    float batteryVoltage() const { return m_batteryVoltage; }
    QString robotIdentity() const { return m_robotIdentity; }

    QBluetoothAddress deviceAddress() const { return m_deviceAddress; }

    static bool isFalconsDevice(const QBluetoothDeviceInfo &device);

public slots:
    void connectToDevice(const QBluetoothDeviceInfo &device);
    void disconnect();

    /** Write a new play state to the robot (0=INVALID, 1=SW_ON, 2=MOT_ON, 3=KICK_ON, 4=INPLAY) */
    void writePlayState(int state);

    /** Write a new WiFi SSID to the robot (triggers wifi switch) */
    void writeWifiSsid(const QString &ssid);

signals:
    void connectionStateChanged();
    void robotNameChanged();
    void robotAddressChanged();
    void rssiChanged();
    void errorOccurred(const QString &error);
    void playStateChanged();
    void wifiSsidChanged();
    void wifiListChanged();
    void batteryVoltageChanged();
    void robotIdentityChanged();

    /** Emitted whenever any robot data is updated (for model refresh) */
    void robotDataUpdated();

private slots:
    void onControllerConnected();
    void onControllerDisconnected();
    void onControllerError(QLowEnergyController::Error error);
    void onServiceDiscovered(const QBluetoothUuid &serviceUuid);
    void onDiscoveryFinished();
    void onServiceStateChanged(QLowEnergyService::ServiceState state);
    void onCharacteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                 const QByteArray &value);
    void onCharacteristicRead(const QLowEnergyCharacteristic &characteristic,
                              const QByteArray &value);

private:
    void setConnectionState(Robot::ConnectionState state);
    void setError(const QString &error);
    void setupService();
    void enableNotifications(const QLowEnergyCharacteristic &characteristic);
    void readAllCharacteristics();
    void parsePlayState(const QByteArray &value);
    void parseWifiSsid(const QByteArray &value);
    void parseWifiList(const QByteArray &value);
    void parseBatteryVoltage(const QByteArray &value);
    void parseRobotIdentity(const QByteArray &value);

    QLowEnergyController *m_controller;
    QLowEnergyService *m_service;

    QLowEnergyCharacteristic m_playStateChar;
    QLowEnergyCharacteristic m_wifiSsidChar;
    QLowEnergyCharacteristic m_wifiListChar;
    QLowEnergyCharacteristic m_batteryVoltageChar;
    QLowEnergyCharacteristic m_robotIdentityChar;

    Robot::ConnectionState m_connectionState;
    QString m_robotName;
    QBluetoothAddress m_deviceAddress;
    int m_rssi;
    QString m_lastError;
    bool m_serviceFound;

    // Cached data from robot
    int m_playState;
    QString m_wifiSsid;
    QStringList m_wifiList;
    float m_batteryVoltage;
    QString m_robotIdentity;
};

#endif // FALCONSROBOTCONNECTION_H
