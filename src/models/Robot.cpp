#include "Robot.h"

Robot::Robot()
    : m_id(0)
    , m_connectionState(Disconnected)
    , m_rssi(-100)
    , m_totalVoltage(0.0f)
    , m_current(0.0f)
    , m_soc(0)
{
}

Robot::Robot(int id, const QString &name, const QBluetoothAddress &address)
    : m_id(id)
    , m_name(name)
    , m_bluetoothAddress(address)
    , m_connectionState(Disconnected)
    , m_rssi(-100)
    , m_totalVoltage(0.0f)
    , m_current(0.0f)
    , m_soc(0)
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
