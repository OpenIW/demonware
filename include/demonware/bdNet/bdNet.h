// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "bdConnection/bdConnection.h"

#include "bdGetHostByName/bdGetHostByNameConfig.h"
#include "bdGetHostByName/bdGetHostByName.h"

#include "bdLANDiscovery/bdLANDiscovery.h"

#include "bdUPnP/bdUPnPConfig.h"
#include "bdUPnP/bdUPnPDevice.h"
#include "bdUPnP/bdUPnP.h"

#include "bdNetStartParams.h"

class bdNetImpl
{
public:
    enum bdNetStatus : bdInt
    {
        BD_NET_PARAMS_CONFIG_ERROR = -4,
        BD_NET_BIND_FAILED = -3,
        BD_NET_ONLINE_FAILED = -2,
        BD_NET_INIT_FAILED = -1,
        BD_NET_STOPPED = 0,
        BD_NET_PENDING = 1,
        BD_NET_DONE = 2,
    };
protected:
    bdNetStartParams m_params;
    bdConnectionStore m_connectionStore;
    bdSocketRouter* m_socketRouter;
    bdNetImpl::bdNetStatus m_status;
    bdIPDiscoveryClient* m_ipDiscClient;
    bdNATTypeDiscoveryClient* m_natTypeDiscClient;
    bdGetHostByName* m_getHostByName;
    bdUPnP m_UPnP;
    bdArray<bdAddr> m_natTravAddrs;
    bdUInt m_currentNatTravAddrIndex;
    bdUInt m_currentNatTravHostIndex;
    bdUInt m_upnpCollisionRetryCount;
public:
    void operator delete(void* p);
    void* operator new (bdUWord nbytes);
    bdNetImpl();
    ~bdNetImpl();
    static bdBool findFreePort(bdAddr& addr);
    bdBool start(const bdNetStartParams& params);
    void pump();
    void stop();
    bdBool receiveAll();
    void dispatchAll();
    bdBool sendAll();
    bdBool getBindAddr(bdAddr& addr);

    const bdNetStartParams& getParams() const;
    bdNetStatus getStatus() const;
    bdSocketRouter* getSocketRouter() const;
    const bdConnectionStore* getConnectionStore() const;
    bdCommonAddrRef getLocalCommonAddr() const;
    bdString getStatusAsString() const;
    bdUPnPDevice::bdUPnPPortStatus getUPnPStatus() const;
};

class bdNet : public bdSingleton<bdNetImpl>
{

};
