#ifndef ROBOT_H
#define ROBOT_H

#include <QObject>
#include <QString>
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

    int rssi() const { return m_rssi; }
    void setRssi(int rssi) { m_rssi = rssi; }

    QByteArray lastPacketReceived() const { return m_lastPacketReceived; }
    void setLastPacketReceived(const QByteArray &packet) { m_lastPacketReceived = packet; }

    QDateTime lastPacketTime() const { return m_lastPacketTime; }
    void setLastPacketTime(const QDateTime &time) { m_lastPacketTime = time; }

    float totalVoltage() const { return m_totalVoltage; }
    void setTotalVoltage(float voltage) { m_totalVoltage = voltage; }

    float current() const { return m_current; }
    void setCurrent(float current) { m_current = current; }

    int soc() const { return m_soc; }
    void setSoc(int soc) { m_soc = soc; }

    static QString connectionStateToString(ConnectionState state);

private:
    int m_id;
    QString m_name;
    QBluetoothAddress m_bluetoothAddress;
    ConnectionState m_connectionState;
    int m_rssi;
    QByteArray m_lastPacketReceived;
    QDateTime m_lastPacketTime;
    float m_totalVoltage;
    float m_current;
    int m_soc;
};

Q_DECLARE_METATYPE(Robot::ConnectionState)

#endif // ROBOT_H
