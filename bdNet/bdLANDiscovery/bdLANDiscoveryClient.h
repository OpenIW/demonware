// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdLANDiscoveryClient
{
public:
    enum bdStatus : bdInt
    {
        BD_IDLE = 0x0,
        BD_UPLOAD = 0x1,
        BD_DOWNLOAD = 0x2,
        BD_DELETE = 0x3,
        BD_COPY = 0x4,
    };
protected:
    bdFloat32 m_timeout;
    bdStopwatch m_timer;
    bdSocket m_socket;
    bdUByte8 m_nonce[8];
    bdFastArray<bdLANDiscoveryListener*> m_listeners;
    bdStatus m_status;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdLANDiscoveryClient();
    ~bdLANDiscoveryClient();
    const bdStatus getStatus() const;
    void registerListener(bdLANDiscoveryListener* listener);
    void stop();
    void unregisterListener(bdLANDiscoveryListener* listener);
    bdBool discover(const bdUInt titleID, const bdFloat32 timeout, const bdInetAddr* addr, const bdUInt16 discoveryPort);
    void update();
};
