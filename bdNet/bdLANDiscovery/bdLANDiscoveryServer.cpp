// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdNet/bdNet.h"

void bdLANDiscoveryServer::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdLANDiscoveryServer::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdLANDiscoveryServer::bdLANDiscoveryServer() : m_gameInfo(), m_socket(), m_listeners(), m_status(BD_IDLE)
{
}

bdLANDiscoveryServer::~bdLANDiscoveryServer()
{
    stop();
}

void bdLANDiscoveryServer::registerListener(bdLANDiscoveryListener* listener)
{
    m_listeners.pushBack(&listener);
}

bdBool bdLANDiscoveryServer::start(const bdReference<bdGameInfo> gameInfo, const bdInetAddr* localAddr, const bdInt16 discoveryPort)
{
    if (!m_socket.create(false, true))
    {
        m_status = BD_DOWNLOAD;
        return false;
    }
    if (m_socket.bind(&bdAddr(localAddr, discoveryPort)) != BD_NET_SUCCESS)
    {
        m_status = BD_DOWNLOAD;
        return false;
    }
    m_status = BD_UPLOAD;
    m_gameInfo = gameInfo;
    return true;
}

void bdLANDiscoveryServer::stop()
{
    m_socket.close();
    m_status = BD_IDLE;
}

void bdLANDiscoveryServer::unregisterListener(bdLANDiscoveryListener* listener)
{
    m_listeners.removeAllKeepOrder(listener);
}

void bdLANDiscoveryServer::update()
{
    bdInt bytesReceived;
    bdUByte8 dataReceived[1288];
    bdBool ok;
    bdUByte8 tempType;
    bdUByte8 tempNonce[8];
    bdNChar8 addrStr[24];
    bdUInt tempTitleID;

    if (!m_status != BD_UPLOAD)
    {
        bdLogWarn("bdNet/discovery server", "Not initialized.");
        return;
    }
    bdAddr tempAddr;
    bytesReceived = m_socket.receiveFrom(&tempAddr, dataReceived, sizeof(dataReceived));
    if (bytesReceived <= 11)
    {
        return;
    }
    bdBitBuffer bitBuffer(dataReceived, CHAR_BIT * bytesReceived, true);
    ok = bitBuffer.readDataType(BD_BB_UNSIGNED_CHAR8_TYPE);
    ok = ok == bitBuffer.readBits(&tempType, CHAR_BIT);
    if (!ok || tempType != 27)
    {
        return;
    }
    ok = bitBuffer.readDataType(BD_BB_FULL_TYPE);
    ok = ok == bitBuffer.readBits(tempNonce, CHAR_BIT * sizeof(tempNonce));
    ok = ok == bitBuffer.readUInt32(&tempTitleID);
    if (!ok)
    {
        return;
    }
    if (m_gameInfo->getTitleID() != tempTitleID)
    {
        return;
    }
    bdBitBuffer replyBitBuffer(0, 0);
    replyBitBuffer.writeDataType(BD_BB_UNSIGNED_CHAR8_TYPE);
    bdUByte8 replyType = 28;
    replyBitBuffer.writeBits(&replyType, CHAR_BIT);
    replyBitBuffer.writeDataType(BD_BB_FULL_TYPE);
    replyBitBuffer.writeBits(tempNonce, CHAR_BIT * sizeof(tempNonce));
    m_gameInfo->serialize(&replyBitBuffer);
    bdInt sendResult = m_socket.sendTo(&tempAddr, replyBitBuffer.getData(), replyBitBuffer.getDataSize());
    if (sendResult <= 0)
    {
        return;
    }
    tempAddr.toString(addrStr, sizeof(addrStr));
    bdLogInfo("bdNet/discovery server", "Sent discovery reply to: %s", addrStr);
    for (bdUInt i = 0; i < m_listeners.getSize(); ++i)
    {
        (*m_listeners[i])->onRequest();
    }
}
