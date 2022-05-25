// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdNATTypeDiscoveryClient
{
public:
    enum bdNATTypeDiscoveryClientState : bdInt
    {
        BD_NTDCS_UNINITIALIZED = 0x0,
        BD_NTDCS_RUN_TEST_1 = 0x1,
        BD_NTDCS_RUN_TEST_2 = 0x2,
        BD_NTDCS_RUN_TEST_3 = 0x3,
        BD_NTDCS_FINI = 0x4,
        BD_NTDCS_ERROR = 0x5,
    };
protected:
    bdSocket* m_socket;
    bdAddr m_serverAddr1;
    bdAddr m_serverAddr2;
    bdAddr m_mappedAddr;
    bdNATType m_NATType;
    bdNATTypeDiscoveryClientState m_state;
    bdUByte8 m_resends;
    bdStopwatch m_timer;
    bdNATTypeDiscoveryConfig m_config;
public:
    bdNATTypeDiscoveryClient();
    bdNATTypeDiscoveryClient(bdNATTypeDiscoveryClient& other);
    ~bdNATTypeDiscoveryClient();
    void* operator new(bdUWord nbytes);
    void operator delete(void* p);
    bdNATType getNATType();
    bdNATTypeDiscoveryClientState getStatus();
    bdBool isRunning();
    bdBool init(bdSocket* socket, const bdAddr& serverAddr, bdNATTypeDiscoveryConfig config);
    void pump(bdAddr fromAddr, const void* data, bdInt dataSize);
    void pumpActiveTest();
    bdBool quit();
    void receiveReplies(bdAddr fromAddr, const void* data, bdInt dataSize);
    bdBool sendForTest1();
    bdBool sendForTest2();
    bdBool sendForTest3();
    bdBool sendNATTypeDiscoveryPacket(bdNATTypeDiscoveryPacket::bdNATTypeDiscoveryPacketRequest packetType, bdAddr& const serverAddr);
    void handleResponse(const bdAddr& addr, const bdNATTypeDiscoveryPacketReply& reply);
};
