// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdLANDiscoveryServer
{
public:
    enum bdStatus
    {
        BD_IDLE = 0,
        BD_PENDING = 1,
        BD_ERROR = 2
    };
protected:
    bdReference<bdGameInfo> m_gameInfo;
    bdSocket m_socket;
    bdFastArray<bdLANDiscoveryListener*> m_listeners;
    bdStatus m_status;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdLANDiscoveryServer();
    ~bdLANDiscoveryServer();
    void registerListener(bdLANDiscoveryListener* listener);
    bdBool start(const bdReference<bdGameInfo> gameInfo, const bdInetAddr& localAddr, const bdInt16 discoveryPort);
    void stop();
    void unregisterListener(bdLANDiscoveryListener* listener);
    void update();
    bdStatus getStatus() const;
};
