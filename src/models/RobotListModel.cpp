#include "RobotListModel.h"

RobotListModel::RobotListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int RobotListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_robots.count();
}

QVariant RobotListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_robots.count())
        return QVariant();

    const Robot &robot = m_robots.at(index.row());

    switch (role) {
    case NameRole:
        return robot.name();
    case AddressRole:
        return robot.bluetoothAddress().toString();
    case ConnectionStateRole:
        return Robot::connectionStateToString(robot.connectionState());
    case RssiRole:
        return robot.rssi();
    case LastDataRole:
        return robot.lastPacketReceived().toHex(':');
    case LastDataTimeRole:
        return robot.lastPacketTime().toString("hh:mm:ss");
    case IdRole:
        return robot.id();
    case TotalVoltageRole:
        return robot.totalVoltage();
    case CurrentRole:
        return robot.current();
    case SocRole:
        return robot.soc();
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> RobotListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[AddressRole] = "address";
    roles[ConnectionStateRole] = "connectionState";
    roles[RssiRole] = "rssi";
    roles[LastDataRole] = "lastData";
    roles[LastDataTimeRole] = "lastDataTime";
    roles[IdRole] = "robotId";
    roles[TotalVoltageRole] = "totalVoltage";
    roles[CurrentRole] = "current";
    roles[SocRole] = "soc";
    return roles;
}

void RobotListModel::addRobot(const Robot &robot)
{
    beginInsertRows(QModelIndex(), m_robots.count(), m_robots.count());
    m_robots.append(robot);
    endInsertRows();
}

void RobotListModel::updateRobot(int index, const Robot &robot)
{
    if (index >= 0 && index < m_robots.count()) {
        m_robots[index] = robot;
        QModelIndex modelIndex = createIndex(index, 0);
        emit dataChanged(modelIndex, modelIndex);
    }
}

void RobotListModel::removeRobot(int index)
{
    if (index >= 0 && index < m_robots.count()) {
        beginRemoveRows(QModelIndex(), index, index);
        m_robots.removeAt(index);
        endRemoveRows();
    }
}

void RobotListModel::clear()
{
    beginResetModel();
    m_robots.clear();
    endResetModel();
}

Robot RobotListModel::robotAt(int index) const
{
    if (index >= 0 && index < m_robots.count())
        return m_robots.at(index);
    return Robot();
}
