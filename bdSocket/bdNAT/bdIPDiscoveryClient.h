// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdIPDiscoveryClient
{
public:
    enum bdIPDiscoveryClientStatus : bdInt
    {
        BD_IP_DISC_UNINITIALIZED = 0x0,
        BD_IP_DISC_RUNNING = 0x1,
        BD_IP_DISC_SUCCESS = 0x2,
        BD_IP_DISC_FAIL = 0x3,
        BD_IP_DISC_ERROR = 0x4,
    };
protected:
    bdSocket* m_socket;
    bdAddr m_serverAddr;
    bdAddr m_publicAddr;
    bdStopwatch m_timer;
    bdIPDiscoveryClientStatus m_status;
    bdIPDiscoveryConfig m_config;
    bdUInt m_retries;
public:
    bdIPDiscoveryClient();
    ~bdIPDiscoveryClient();
    void* operator new(bdUWord nbytes);
    void operator delete(void* p);
    bdBool init(bdSocket* socket, const bdAddr* server, bdIPDiscoveryConfig config);
    bdBool sendIPDiscoveryPacket();
    const bdAddr* getPublicAddress();
    void pump(bdAddr fromAddr, bdUByte8* data, bdUInt dataSize);
    void quit();
    bdUInt getStatus();
};