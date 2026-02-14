#include "PacketInterface.h"

PacketInterface::PacketInterface(QObject *parent)
    : QObject(parent)
{
}

void PacketInterface::sendPacket(const QByteArray &data)
{
    // For now, just pass through the raw data
    emit packetToSend(data);
}

void PacketInterface::onDataReceived(const QByteArray &data)
{
    // For now, just pass through the raw data
    emit packetReceived(data);
}
