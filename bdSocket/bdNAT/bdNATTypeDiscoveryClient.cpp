// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocket.h"

bdNATTypeDiscoveryClient::bdNATTypeDiscoveryClient()
{
    m_NATType = BD_NAT_UNKNOWN;
    m_state = BD_NTDCS_UNINITIALIZED;
    m_resends = 0;
}

bdNATTypeDiscoveryClient::~bdNATTypeDiscoveryClient()
{
}

void* bdNATTypeDiscoveryClient::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

void bdNATTypeDiscoveryClient::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

bdNATType bdNATTypeDiscoveryClient::getNATType()
{
    if (m_state == BD_NTDCS_FINI)
    {
        return m_NATType;
    }
    return BD_NAT_UNKNOWN;
}

bdNATTypeDiscoveryClient::bdNATTypeDiscoveryClientState bdNATTypeDiscoveryClient::getStatus()
{
    return m_state;
}

bdBool bdNATTypeDiscoveryClient::isRunning()
{
    if (m_state == BD_NTDCS_RUN_TEST_1 || m_state == BD_NTDCS_RUN_TEST_2 || m_state == BD_NTDCS_RUN_TEST_3)
    {
        return true;
    }
    return false;
}

bdBool bdNATTypeDiscoveryClient::init(bdSocket* socket, const bdAddr& serverAddr, bdNATTypeDiscoveryConfig config)
{
    if (m_state)
    {
        bdLogWarn("bdSocket/nat", "Cannot initialize already initialized class.");
        return false;
    }
    else if (socket)
    {
        m_socket = socket;
        bdLogInfo("bdSocket/nat", "NAT discovery client initialized");
        bdMemcpy(&m_serverAddr1, &serverAddr, sizeof(bdAddr));
        m_config = config;
        m_config.sanityCheckConfig();
        if (sendForTest1())
        {
            m_state = BD_NTDCS_RUN_TEST_1;
            m_timer.start();
            return true;
        }
        m_state = BD_NTDCS_ERROR;
        return true;
    }
    else
    {
        bdLogWarn("bdSocket/nat", "Must initialize NAT Discovery Client with a valid socket");
        m_state = BD_NTDCS_ERROR;
        return false;
    }
}

void bdNATTypeDiscoveryClient::pump(bdAddr fromAddr, const void* data, bdInt dataSize)
{
    if (isRunning())
    {
        receiveReplies(bdAddr(fromAddr), data, dataSize);
        pumpActiveTest();
    }
    else
    {
        bdLogWarn("bdSocket/nat", "This class does not need to be pumped unless NAT type detection is running");
    }
}

void bdNATTypeDiscoveryClient::pumpActiveTest()
{
    switch (m_state)
    {
    case BD_NTDCS_UNINITIALIZED:
        bdLogWarn("bdSocket/nat", "Code logic error in NTDC pump");
        break;
    case BD_NTDCS_RUN_TEST_1:
        if (m_timer.getElapsedTimeInSeconds() > m_config.m_NtdcsSendTimeout)
        {
            ++m_resends;
            if (m_resends < m_config.m_NtdcsMaxResends)
            {
                m_timer.start();
                if (!sendForTest1())
                {
                    m_state = BD_NTDCS_ERROR;
                }
                break;
            }
            bdLogInfo("bdSocket/nat", "Test 1 failed. Not online.");
            m_state = BD_NTDCS_ERROR;
        }
        break;
    case BD_NTDCS_RUN_TEST_2:
        if (m_timer.getElapsedTimeInSeconds() > m_config.m_NtdcsSendTimeout)
        {
            ++m_resends;
            if (m_resends < m_config.m_NtdcsMaxResends)
            {
                m_timer.start();
                if (!sendForTest2())
                {
                    m_state = BD_NTDCS_ERROR;
                }
                break;
            }
            bdLogInfo("bdSocket/nat", "Test 2 failed.");
            m_resends = 0;
            m_state = BD_NTDCS_RUN_TEST_3;
        }
        break;
    case BD_NTDCS_RUN_TEST_3:
        if (m_timer.getElapsedTimeInSeconds() > m_config.m_NtdcsSendTimeout)
        {
            ++m_resends;
            if (m_resends < m_config.m_NtdcsMaxResends)
            {
                m_timer.start();
                if (!sendForTest3())
                {
                    m_state = BD_NTDCS_ERROR;
                }
                break;
            }
            bdLogInfo("bdSocket/nat", "Test 3 failed. Strict NAT.");
            m_NATType = BD_NAT_STRICT;
            m_state = BD_NTDCS_FINI;
        }
        break;
    default:
        return;
    }
}

bdBool bdNATTypeDiscoveryClient::quit()
{
    if (m_state)
    {
        bdLogInfo("bdSocket/nat", "NAT discovery client quit");
        m_socket = NULL;
        m_state = BD_NTDCS_UNINITIALIZED;
        return true;
    }
    bdLogWarn("bdSocket/nat", "Cannot quit uninitialized class.");
    return false;
}

void bdNATTypeDiscoveryClient::receiveReplies(bdAddr fromAddr, const void* data, bdInt dataSize)
{
    bdUInt tmpUInt;

    if (dataSize > 0)
    {
        bdNATTypeDiscoveryPacketReply reply;
        if (reply.deserialize(data, dataSize, 0, tmpUInt) && reply.getType() == 21)
        {
            handleResponse(fromAddr, reply);
        }
    }
}

bdBool bdNATTypeDiscoveryClient::sendForTest1()
{
    if (sendNATTypeDiscoveryPacket(bdNATTypeDiscoveryPacket::BD_NTDP_SAME_ADDR, m_serverAddr1))
    {
        bdLogInfo("bdSocket/nat", "Sent packet for NTDC test 1");
        return true;
    }
    return false;
}

bdBool bdNATTypeDiscoveryClient::sendForTest2()
{
    if (sendNATTypeDiscoveryPacket(bdNATTypeDiscoveryPacket::BD_NTDP_DIFF_PORT_IP, m_serverAddr1))
    {
        bdLogInfo("bdSocket/nat", "Sent packet for NTDC test 2");
        return true;
    }
    return false;
}

bdBool bdNATTypeDiscoveryClient::sendForTest3()
{
    if (sendNATTypeDiscoveryPacket(bdNATTypeDiscoveryPacket::BD_NTDP_DIFF_IP, m_serverAddr2))
    {
        bdLogInfo("bdSocket/nat", "Sent packet for NTDC test 3");
        return true;
    }
    return false;
}

bdBool bdNATTypeDiscoveryClient::sendNATTypeDiscoveryPacket(bdNATTypeDiscoveryPacket::bdNATTypeDiscoveryPacketRequest packetType, bdAddr& serverAddr)
{
    bdInt lenDataSent;
    bdUByte8 buffer[1288];
    bdNATTypeDiscoveryPacket packet(packetType);
    bdUInt len = 0;

    if (packet.serialize(buffer, sizeof(buffer), 0, len))
    {
        lenDataSent = m_socket->sendTo(serverAddr, buffer, len);
        if (lenDataSent <= 0)
        {
            bdLogError("bdSocket/nat", "Failed to send NTDC packet");
            return false;
        }
        if (lenDataSent != len)
        {
            bdLogError("bdSocket/nat", "Sent packet length does not match packet length !");
            return false;
        }
        return true;
    }
    bdLogError("bdSocket/nat", "Failed to serialize NTDC packet.");
    return false;
}

void bdNATTypeDiscoveryClient::handleResponse(const bdAddr& addr, const bdNATTypeDiscoveryPacketReply& reply)
{
    bdBool portMatch;
    bdBool addressMatch;
    bdNChar8 expectedAddrString[22];
    bdNChar8 receivedAddrString[22];

    switch (m_state)
    {
    case BD_NTDCS_RUN_TEST_3:
        if (m_mappedAddr == reply.getMappedAddr())
        {
            bdLogInfo("bdSocket/nat", "Reply for test 3. Moderate NAT.");
            m_state = BD_NTDCS_FINI;
            m_NATType = BD_NAT_MODERATE;
        }
        else
        {
            bdLogInfo("bdSocket/nat", "Reply for test 3. Strict NAT.");
            m_state = BD_NTDCS_FINI;
            m_NATType = BD_NAT_STRICT;
        }
        break;
    case BD_NTDCS_RUN_TEST_2:
        addressMatch = addr.getAddress() == m_serverAddr2.getAddress();
        portMatch = addr.getPort() == m_serverAddr2.getPort();
        if (!addressMatch || portMatch)
        {
            if (!addressMatch)
            {
                addr.toString(receivedAddrString, sizeof(receivedAddrString));
                m_serverAddr2.toString(expectedAddrString, sizeof(expectedAddrString));
                bdLogWarn("bdSocket/nat", "Received test 2 response from unexpected address \n received from : %s \n expected from :%s ", receivedAddrString, expectedAddrString);
            }
            if (portMatch)
            {
                bdLogWarn("bdSocket/nat", "Received test 2 response from primary port");
            }
        }
        else
        {
            bdLogInfo("bdSocket/nat", "Reply for test 2. Open NAT (or no NAT).");
            m_state = BD_NTDCS_FINI;
            m_NATType = BD_NAT_OPEN;
        }
        break;
    case BD_NTDCS_RUN_TEST_1:
        bdMemcpy(&m_serverAddr2, &reply.getSecAddr(), sizeof(m_serverAddr2));
        bdMemcpy(&m_mappedAddr, &reply.getMappedAddr(), sizeof(m_mappedAddr));
        bdLogInfo("bdSocket/nat", "Reply for test 1. Start test 2.");
        if (sendForTest2())
        {
            m_state = BD_NTDCS_RUN_TEST_2;
            m_resends = 0;
        }
        else
        {
            m_state = BD_NTDCS_ERROR;
        }
        break;

    }
}
