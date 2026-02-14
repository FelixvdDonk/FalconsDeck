#ifndef PACKETINTERFACE_H
#define PACKETINTERFACE_H

#include <QObject>
#include <QByteArray>

class PacketInterface : public QObject
{
    Q_OBJECT

public:
    explicit PacketInterface(QObject *parent = nullptr);

    void sendPacket(const QByteArray &data);

signals:
    void packetReceived(const QByteArray &data);
    void packetToSend(const QByteArray &data);

public slots:
    void onDataReceived(const QByteArray &data);
};

#endif // PACKETINTERFACE_H
