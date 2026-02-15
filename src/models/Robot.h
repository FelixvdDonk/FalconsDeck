#ifndef ROBOT_H
#define ROBOT_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QBluetoothAddress>
#include <QDateTime>
#include <QByteArray>

class Robot
{
public:
    enum ConnectionState {
        Disconnected,
        Connecting,
        Connected,
        Ready,
        Error
    };

    // Device type — determines which UI and protocol to use
    enum DeviceType {
        Unknown,
        FalconsRobot,   // Falcons football robot (custom GATT service)
        SmartBMS        // JBD/SmartBMS battery monitor
    };

    Robot();
    Robot(int id, const QString &name, const QBluetoothAddress &address);

    int id() const { return m_id; }
    void setId(int id) { m_id = id; }

    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }

    QBluetoothAddress bluetoothAddress() const { return m_bluetoothAddress; }
    void setBluetoothAddress(const QBluetoothAddress &address) { m_bluetoothAddress = address; }

    ConnectionState connectionState() const { return m_connectionState; }
    void setConnectionState(ConnectionState state) { m_connectionState = state; }

    DeviceType deviceType() const { return m_deviceType; }
    void setDeviceType(DeviceType type) { m_deviceType = type; }

    int rssi() const { return m_rssi; }
    void setRssi(int rssi) { m_rssi = rssi; }

    QByteArray lastPacketReceived() const { return m_lastPacketReceived; }
    void setLastPacketReceived(const QByteArray &packet) { m_lastPacketReceived = packet; }

    QDateTime lastPacketTime() const { return m_lastPacketTime; }
    void setLastPacketTime(const QDateTime &time) { m_lastPacketTime = time; }

    // ── BMS data ──
    float totalVoltage() const { return m_totalVoltage; }
    void setTotalVoltage(float voltage) { m_totalVoltage = voltage; }

    float current() const { return m_current; }
    void setCurrent(float current) { m_current = current; }

    int soc() const { return m_soc; }
    void setSoc(int soc) { m_soc = soc; }

    QList<float> cellVoltages() const { return m_cellVoltages; }
    void setCellVoltages(const QList<float> &voltages) { m_cellVoltages = voltages; }

    // ── Falcons Robot data ──
    int playState() const { return m_playState; }
    void setPlayState(int state) { m_playState = state; }

    QString wifiSsid() const { return m_wifiSsid; }
    void setWifiSsid(const QString &ssid) { m_wifiSsid = ssid; }

    QStringList wifiList() const { return m_wifiList; }
    void setWifiList(const QStringList &list) { m_wifiList = list; }

    float batteryVoltage() const { return m_batteryVoltage; }
    void setBatteryVoltage(float voltage) { m_batteryVoltage = voltage; }

    QString robotIdentity() const { return m_robotIdentity; }
    void setRobotIdentity(const QString &identity) { m_robotIdentity = identity; }

    static QString connectionStateToString(ConnectionState state);
    static QString playStateToString(int state);
    static QString deviceTypeToString(DeviceType type);

private:
    int m_id;
    QString m_name;
    QBluetoothAddress m_bluetoothAddress;
    ConnectionState m_connectionState;
    DeviceType m_deviceType;
    int m_rssi;
    QByteArray m_lastPacketReceived;
    QDateTime m_lastPacketTime;

    // BMS data
    float m_totalVoltage;
    float m_current;
    int m_soc;
    QList<float> m_cellVoltages;

    // Falcons Robot data
    int m_playState;
    QString m_wifiSsid;
    QStringList m_wifiList;
    float m_batteryVoltage;
    QString m_robotIdentity;
};

Q_DECLARE_METATYPE(Robot::ConnectionState)

#endif // ROBOT_H
