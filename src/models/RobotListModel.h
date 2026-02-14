#ifndef ROBOTLISTMODEL_H
#define ROBOTLISTMODEL_H

#include <QAbstractListModel>
#include <QList>
#include "Robot.h"

class RobotListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum RobotRoles {
        NameRole = Qt::UserRole + 1,
        AddressRole,
        ConnectionStateRole,
        RssiRole,
        LastDataRole,
        LastDataTimeRole,
        IdRole,
        TotalVoltageRole,
        CurrentRole,
        SocRole,
        CellVoltagesRole
    };

    explicit RobotListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addRobot(const Robot &robot);
    void updateRobot(int index, const Robot &robot);
    void removeRobot(int index);
    void clear();

    Robot robotAt(int index) const;
    int count() const { return m_robots.count(); }

private:
    QList<Robot> m_robots;
};

#endif // ROBOTLISTMODEL_H
