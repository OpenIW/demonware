// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocket.h"

bdIPDiscoveryClient::bdIPDiscoveryClient()
{
    m_socket = NULL;
    m_status = BD_IP_DISC_UNINITIALIZED;
    m_retries = 0;
}

bdIPDiscoveryClient::~bdIPDiscoveryClient()
{
    m_status = BD_IP_DISC_UNINITIALIZED;
    m_socket = NULL;
}

void* bdIPDiscoveryClient::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

void bdIPDiscoveryClient::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

bdBool bdIPDiscoveryClient::init(bdSocket* socket, const bdAddr& server, bdIPDiscoveryConfig config)
{
    if (m_status)
    {
        bdLogWarn("bdSocket/nat", "init() called multiple times.");
        return false;
    }
    if (socket)
    {
        m_socket = socket;
        bdMemcpy(&m_serverAddr, &server, sizeof(m_serverAddr));
        m_retries = 0;
        config.sanityCheckConfig();
        m_config = config;
        m_status = BD_IP_DISC_RUNNING;
        if (sendIPDiscoveryPacket())
        {
            return true;
        }
        return false;
    }
    bdLogWarn("bdSocket/nat", "init() called multiple times.");
    return false;
}

void bdIPDiscoveryClient::pump(bdAddr fromAddr, bdUByte8* data, bdUInt dataSize)
{
    bdUInt retries;
    bdUInt tmpUInt;
    bdNChar8 addrBuffer[100];
    bdNChar8 protectedAddrBuffer[100];
    bdNChar8 publicAddrBuffer[100];

    if (m_status != BD_IP_DISC_RUNNING)
    {
        return;
    }
    if (dataSize > 0)
    {
        bdIPDiscoveryPacketReply packet;
        if (&fromAddr == &m_serverAddr && packet.deserialize(data, dataSize, 0, tmpUInt))
        {
            bdMemcpy(&m_publicAddr, &packet.getAddr(), sizeof(m_publicAddr));
            m_status = BD_IP_DISC_SUCCESS;
            m_publicAddr.toString(addrBuffer, sizeof(addrBuffer));
            bdLogInfo("bdSocket/nat", "Public IP discovered: %s", addrBuffer);
        }
    }
    if (m_status == BD_IP_DISC_RUNNING)
    {
        if (m_config.m_requestTimeout >= m_timer.getElapsedTimeInSeconds() || (m_retries++, m_retries - 1 >= m_config.m_retries))
        {
            if (m_retries >= m_config.m_retries)
            {
                bdLogError("bdSocket/nat", "IP Discovery failed with %u retries, giving up. ");
                m_status = BD_IP_DISC_FAIL;
            }
        }
        else
        {
            bdLogWarn("bdSocket/nat", "IP Discovery failed. Retrying. (%u/%u)", m_retries, m_config.m_retries);
            sendIPDiscoveryPacket();
        }
    }
}

void bdIPDiscoveryClient::quit()
{
    m_socket = NULL;
    bdMemcpy(&m_serverAddr, &bdAddr(), sizeof(m_serverAddr));
    bdMemcpy(&m_publicAddr, &bdAddr(), sizeof(m_publicAddr));
    m_timer.reset();
    m_status = BD_IP_DISC_UNINITIALIZED;
    m_retries = 0;
}

bdUInt bdIPDiscoveryClient::getStatus()
{
    return m_status;
}

const bdAddr* bdIPDiscoveryClient::getPublicAddress()
{
    if (m_status != BD_IP_DISC_SUCCESS)
    {
        bdLogWarn("bdSocket/nat", "bdIPDiscoveryClient::getPublicAddress() called, but ip discovery was not successful (or not yet finished).");
    }
    return &m_publicAddr;
}

bdBool bdIPDiscoveryClient::sendIPDiscoveryPacket()
{
    bdInt sendResult;
    bdUByte8 buffer[1288];

    if (m_status != BD_IP_DISC_RUNNING)
    {
        return false;
    }
    m_timer.start();
    bdIPDiscoveryPacket packet;
    bdMemset(buffer, 0, sizeof(buffer));
    bdUInt len = 0;
    if (!packet.serialize(buffer, sizeof(buffer), 0, len))
    {
        bdLogError("bdSocket/nat", "Failed to serialize IP Discovery packet.");
        m_status = BD_IP_DISC_ERROR;
        return false;
    }
    sendResult = m_socket->sendTo(m_serverAddr, buffer, len);
    if (sendResult <= 0 || sendResult != len)
    {
        bdLogError("bdSocket/nat", "Failed to send IP discovery packet.");
        m_status = BD_IP_DISC_ERROR;
        return false;
    }
    return true;
}
