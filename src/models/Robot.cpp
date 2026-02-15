#include "Robot.h"

Robot::Robot()
    : m_id(0)
    , m_connectionState(Disconnected)
    , m_deviceType(Unknown)
    , m_rssi(-100)
    , m_totalVoltage(0.0f)
    , m_current(0.0f)
    , m_soc(0)
    , m_playState(0)
    , m_batteryVoltage(0.0f)
{
}

Robot::Robot(int id, const QString &name, const QBluetoothAddress &address)
    : m_id(id)
    , m_name(name)
    , m_bluetoothAddress(address)
    , m_connectionState(Disconnected)
    , m_deviceType(Unknown)
    , m_rssi(-100)
    , m_totalVoltage(0.0f)
    , m_current(0.0f)
    , m_soc(0)
    , m_playState(0)
    , m_batteryVoltage(0.0f)
{
}

QString Robot::connectionStateToString(ConnectionState state)
{
    switch (state) {
    case Disconnected:
        return QStringLiteral("Disconnected");
    case Connecting:
        return QStringLiteral("Connecting");
    case Connected:
        return QStringLiteral("Connected");
    case Ready:
        return QStringLiteral("Ready");
    case Error:
        return QStringLiteral("Error");
    default:
        return QStringLiteral("Unknown");
    }
}

QString Robot::playStateToString(int state)
{
    switch (state) {
    case 0:  return QStringLiteral("Off");
    case 1:  return QStringLiteral("Software On");
    case 2:  return QStringLiteral("Motors On");
    case 3:  return QStringLiteral("Kicker On");
    case 4:  return QStringLiteral("In Play");
    default: return QStringLiteral("Unknown");
    }
}

QString Robot::deviceTypeToString(DeviceType type)
{
    switch (type) {
    case FalconsRobot: return QStringLiteral("FalconsRobot");
    case SmartBMS:     return QStringLiteral("SmartBMS");
    default:           return QStringLiteral("Unknown");
    }
}
