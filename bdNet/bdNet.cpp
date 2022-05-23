// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdNet/bdNet.h"

bdNetImpl* bdSingleton<bdNetImpl>::m_instance = NULL;

void bdNetImpl::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdNetImpl::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdNetImpl::bdNetImpl()
    : m_params(), m_connectionStore(), m_socketRouter(NULL), m_status(BD_NET_STOPPED), m_ipDiscClient(NULL), m_natTypeDiscClient(NULL), m_getHostByName(NULL),
    m_UPnP(), m_natTravAddrs(0u), m_currentNatTravAddrIndex(0), m_currentNatTravHostIndex(0), m_upnpCollisionRetryCount(0)
{
}

bdNetImpl::~bdNetImpl()
{
    if (m_getHostByName)
    {
        m_getHostByName->quit();
        if (m_getHostByName)
        {
            m_getHostByName->~bdGetHostByName();
        }
        m_getHostByName = NULL;
    }
    if (m_natTypeDiscClient)
    {
        m_natTypeDiscClient->quit();
        if (m_natTypeDiscClient)
        {
            m_natTypeDiscClient->~bdNATTypeDiscoveryClient();
        }
        m_natTypeDiscClient = NULL;
    }
    if (m_ipDiscClient)
    {
        m_ipDiscClient->quit();
        if (m_ipDiscClient)
        {
            m_ipDiscClient->~bdIPDiscoveryClient();
        }
        m_ipDiscClient = NULL;
    }
    if (m_socketRouter)
    {
        m_socketRouter->quit();
        if (m_socketRouter)
        {
            m_socketRouter->~bdSocketRouter();
        }
        m_socketRouter = NULL;
    }
    delete &m_natTravAddrs;
    delete &m_UPnP;
    delete &m_connectionStore;
    delete &m_params;
}

bdBool bdNetImpl::findFreePort(bdNetStartParams* params, bdAddr* addr)
{
    bdSocketStatusCode bindResult;
    bdPort basePort = addr->getPort();
    bdPort port = basePort;
    bdBool success = false;

    if (!params->m_socket)
    {
        params->m_socket = new bdSocket();
    }
    for (bdUInt i = 0; i < 100; ++i)
    {
        if (params->m_socket->create(false, true) == 0)
        {
            bdLogWarn("bdNet/net", "Create socket failed ");
            break;
        }
        bdAddr tryAddr(addr->getAddress(), port);
        bindResult = params->m_socket->bind(&tryAddr);
        if (bindResult == BD_NET_SUCCESS)
        {
            params->m_gamePort = tryAddr.getPort();
            bdLogInfo("bdNet/net", "Requested port %u, using port %u", port, params->m_gamePort);
            success = true;
            break;
        }
        else if (bindResult == BD_NET_ADDRESS_IN_USE && (params->m_socket->close() & 1) != 0)
        {
            bdLogWarn("bdNet/net", "Socket bind failed, but subsequent close succeeded!");
        }
        else
        {
            bdLogWarn("bdNet/net", "Socket failed to close with an error");
        }
        port = bdRandom().nextUInt() % 100 + basePort;
    }
    return success;
}

bdBool bdNetImpl::start(const bdNetStartParams* params)
{
    if (m_status != BD_NET_STOPPED)
    {
        bdLogWarn("bdNet/net", "bdNet can only be started when in the uninitialized state. Call stop() to reset");
        return false;
    }
    m_params = params;
    bdAddr addr;
    if (!getBindAddr(&addr))
    {
        bdLogWarn("bdNet/net", "Failed to configure bind address");
        m_status = BD_NET_PARAMS_CONFIG_ERROR;
        return false;
    }
    if (!findFreePort(&m_params, &addr))
    {
        bdLogWarn("bdNet/net", "Failed to find a free port");
        m_status = BD_NET_BIND_FAILED;
        return false;
    }
    m_socketRouter = new bdSocketRouter();
    if (!m_params.m_onlineGame)
    {
        m_status = BD_NET_DONE;
        if (m_params.m_localAddresses.isEmpty())
        {
            bdLogWarn("bdNet/net", "No local address specified when starting bdNet. See bdNetStartParams.");
            return false;
        }
        bdArray<bdAddr> localAddresses(0u);
        for (bdUInt i = 0; i < m_params.m_localAddresses.getSize(); ++i)
        {
            localAddresses.pushBack(&bdAddr(m_params.m_localAddresses[i], m_params.m_gamePort));
        }
        bdAddr remAddr;
        bdCommonAddrRef localCommonAddr(new bdCommonAddr(&localAddresses, &remAddr, BD_NAT_OPEN));
        m_socketRouter->init(m_params.m_socket, &bdCommonAddrRef(&localCommonAddr), &bdSocketRouterConfig());
        m_connectionStore.init(m_socketRouter, &bdConnectionStoreConfig());
    }
    else
    {
        m_status = BD_NET_PENDING;
        m_currentNatTravAddrIndex = 0;
        m_currentNatTravHostIndex = 0;
        m_getHostByName = new bdGetHostByName();
        if (!m_getHostByName->start(m_params.m_natTravHosts[m_currentNatTravHostIndex]->getBuffer(), m_params.m_hostNameLookupConfig))
        {
            bdLogWarn("bdNet/net", "Failed to start DNS lookup task.");
            m_status = BD_NET_INIT_FAILED;
            return false;
        }
        if (!m_UPnP.init(m_params.m_gamePort, &m_params.m_localAddresses, &bdUPnPConfig(&m_params.m_UPnPConfig)))
        {
            bdLogError("bdNet/net", "UPnP module failed to start.");
            return false;
        }
    }
    return true;
}

void bdNetImpl::pump()
{
    bdNChar8 addrBuffer[22];
    bdUByte8 data[1288];

    if (m_params.m_onlineGame || m_status == BD_NET_PENDING)
    {
        return;
    }
    bdBool finishedIPLookup = false;
    bdBool finishedNATLookup = false;
    bdUInt numNATHosts = m_params.m_natTravHosts.getSize();
    bdBool finishedNATHostLookup = m_currentNatTravHostIndex == numNATHosts;

    // Check if UPnP is running, if it is pump it. Else, check the state and port status and start.
    if (m_UPnP.isRunning())
    {
        m_UPnP.pump();
    }
    else
    {
        if (m_UPnP.getState() == bdUPnP::BD_UPNP_FINISHED && m_UPnP.getPortStatus() == bdUPnPDevice::BD_UPNP_PORT_COLLISION_FOUND)
        {
            if (m_upnpCollisionRetryCount <= 20)
            {
                bdNetStartParams params(&m_params);
                params.m_socket = NULL;
                bdLogInfo("bdNet/net", "UPnP Mapping collision found. New port Selected: %d", ++params.m_gamePort);
                bdSingleton<bdNetImpl>::getInstance()->stop();
                start(&params);
            }
            else
            {
                m_UPnP.startShutdown(bdUPnPDevice::BD_UPNP_DEVICE_SHUTDOWN_IMMEDIATE);
                bdLogWarn("bdNet/net", "UPnP stopping after %u port collisions.", m_upnpCollisionRetryCount);
                bdNetStartParams params(&m_params);
                params.m_socket = NULL;
                params.m_gamePort = 3074;
                params.m_UPnPConfig.m_runMode = bdUPnPConfig::BD_UPNP_EXTERNAL_IP_ONLY;
                bdSingleton<bdNetImpl>::getInstance()->stop();
                start(&params);
            }
        }
    }

    if (m_getHostByName)
    {
        bdGetHostByName::bdStatus status = m_getHostByName->getStatus();
        if (status == bdGetHostByName::BD_LOOKUP_PENDING)
        {
            m_getHostByName->pump();
        }
        else if (status = bdGetHostByName::BD_LOOKUP_SUCCEEDED)
        {
            bdUInt numAddrs = m_getHostByName->getNumAddresses();
            for (bdUInt i = 0; i < numAddrs; ++i)
            {
                bdInetAddr inAddr(m_getHostByName->getAddressAt(i)->inUn.m_iaddr);
                bdAddr addr(&inAddr, m_params.m_natTravPort);
                bdUInt index = 0;
                addr.toString(addrBuffer, sizeof(addrBuffer));
                if (m_natTravAddrs.findFirst(&addr, &index))
                {
                    bdLogInfo("bdNet/net", "Duplicate IP %s for Host: %s ignored", addrBuffer, m_params.m_natTravHosts[m_currentNatTravHostIndex]->getBuffer());
                }
                else
                {
                    bdLogInfo("bdNet/net", "Adding IP %s for Host: %s", addrBuffer);
                    m_natTravAddrs.pushBack(&addr);
                }
            }
            ++m_currentNatTravHostIndex;
        }
        else
        {
            bdLogWarn("bdNet/net", "No IP found for Host: %s", m_params.m_natTravHosts[m_currentNatTravHostIndex]->getBuffer());
            ++m_currentNatTravHostIndex;
        }

        if (m_currentNatTravHostIndex < numNATHosts && status != bdGetHostByName::BD_LOOKUP_PENDING)
        {
            m_getHostByName->quit();
            if (!m_getHostByName->start(m_params.m_natTravHosts[m_currentNatTravHostIndex]->getBuffer(), bdGetHostByNameConfig(m_params.m_hostNameLookupConfig.m_timeout)))
            {
                bdLogWarn("bdNet/net", "Failed to start DNS lookup task.");
                m_status = BD_NET_INIT_FAILED;
            }
        }
        if (m_currentNatTravHostIndex == numNATHosts)
        {
            m_getHostByName->quit();
            if (m_getHostByName)
            {
                m_getHostByName->~bdGetHostByName();
            }
            m_getHostByName = NULL;
        }
    }

    if (!m_getHostByName && !m_UPnP.isRunning() && m_UPnP.getPortStatus() != bdUPnPDevice::BD_UPNP_PORT_COLLISION_FOUND && !m_ipDiscClient && !m_natTypeDiscClient)
    {
        if (!m_natTravAddrs.isEmpty())
        {
            bdAddr addr(m_natTravAddrs[m_currentNatTravAddrIndex]);
            m_ipDiscClient = new bdIPDiscoveryClient();
            bdBool startIPDiscoveryResult = m_ipDiscClient->init(m_params.m_socket, &addr, bdIPDiscoveryConfig());
            m_natTypeDiscClient = new bdNATTypeDiscoveryClient();
            bdBool startNATDiscoveryResult = m_natTypeDiscClient->init(m_params.m_socket, &addr, bdNATTypeDiscoveryConfig());
            if (!startIPDiscoveryResult)
            {
                bdLogWarn("bdNet/net", "Failed to start IP discovery");
                m_status = BD_NET_ONLINE_FAILED;
            }
            if (!startNATDiscoveryResult)
            {
                bdLogWarn("bdNet/net", "Failed to start NAT type discovery");
                m_status = BD_NET_ONLINE_FAILED;
            }
        }
        else
        {
            for (bdUInt i = 0; i < m_params.m_natTravHosts.getSize(); ++i)
            {
                bdLogWarn("bdNet/net", "Failed to lookup address %s", m_params.m_natTravHosts[i]->getBuffer());
            }
        }
    }
    bdAddr fromAddr;
    bdInt recieved = m_params.m_socket->receiveFrom(&fromAddr, data, sizeof(data));
    if (m_ipDiscClient)
    {
        m_ipDiscClient->pump(bdAddr(&fromAddr), data, recieved);
        if (m_ipDiscClient->getStatus() != bdIPDiscoveryClient::BD_IP_DISC_RUNNING)
        {
            finishedIPLookup = true;
        }
    }
    if (m_natTypeDiscClient)
    {
        m_natTypeDiscClient->pump(bdAddr(&fromAddr), data, recieved);
        if (!m_natTypeDiscClient->isRunning())
        {
            finishedNATLookup = true;
        }
    }
    if (finishedNATHostLookup && finishedIPLookup && finishedNATLookup)
    {
        if (m_ipDiscClient->getStatus() == bdIPDiscoveryClient::BD_IP_DISC_SUCCESS && m_natTypeDiscClient->getStatus() == bdNATTypeDiscoveryClient::BD_NTDCS_FINI)
        {
            bdArray<bdAddr> localAddresses(1);
            for (bdUInt i = 0; i < m_params.m_localAddresses.getSize(); ++i)
            {
                localAddresses.pushBack(&bdAddr(m_params.m_localAddresses[i], m_params.m_gamePort));
            }
            bdAddr publicAddr(m_ipDiscClient->getPublicAddress());
            bdNATType clientNAT = m_natTypeDiscClient->getNATType();
            bdAddr UPnPExternal;
            if (m_UPnP.getExternalAddr(&UPnPExternal))
            {
                if (UPnPExternal == &publicAddr)
                {
                    bdLogInfo("bdNet/net", "UPnP mapping confirmed.");
                }
                else
                {
                    bdLogInfo("bdNet/net", "External IP detected through STUN server does not match the one detected through UPnP. Possible Nested NATs.");
                }
            }
            bdCommonAddrRef localCommonAddr(new bdCommonAddr(&localAddresses, &publicAddr, clientNAT));
            if (m_ipDiscClient)
            {
                m_ipDiscClient->quit();
                if (m_ipDiscClient)
                {
                    m_ipDiscClient->~bdIPDiscoveryClient();
                    delete m_ipDiscClient;
                }
                m_ipDiscClient = NULL;
            }
            if (m_natTypeDiscClient)
            {
                m_natTypeDiscClient->quit();
                if (m_natTypeDiscClient)
                {
                    m_natTypeDiscClient->~bdNATTypeDiscoveryClient();
                    delete m_natTypeDiscClient;
                }
                m_natTypeDiscClient = NULL;
            }
            bdArray<bdAddr> introducers(&m_natTravAddrs);
            if (m_currentNatTravAddrIndex)
            {
                bdAddr tmpAddr(m_natTravAddrs[0]);
                *introducers[0] = m_natTravAddrs[m_currentNatTravAddrIndex];
                *introducers[m_currentNatTravAddrIndex] = &tmpAddr;
            }
            m_socketRouter->init(m_params.m_socket, bdCommonAddrRef(&localCommonAddr), &bdSocketRouterConfig());
            m_socketRouter->setupIntroducers(&introducers);
            m_connectionStore.init(m_socketRouter, &bdConnectionStoreConfig());
        }
        else
        {
            if (m_currentNatTravAddrIndex + 1 < m_natTravAddrs.getSize())
            {
                bdLogInfo("bdNet/net", "Trying next IP address for IP and NAT discovery.");
                bdAddr addr(m_natTravAddrs[++m_currentNatTravAddrIndex]);

                // quit and cleanup the old IPDiscoveryClient
                if (m_ipDiscClient)
                {
                    m_ipDiscClient->quit();
                    if (m_ipDiscClient)
                    {
                        m_ipDiscClient->~bdIPDiscoveryClient();
                        delete m_ipDiscClient;
                    }
                    m_ipDiscClient = NULL;
                }
                // init a new IPDiscoveryClient with the new IP
                m_ipDiscClient = new bdIPDiscoveryClient();
                bdBool startIPDiscoveryResult = m_ipDiscClient->init(m_params.m_socket, &addr, bdIPDiscoveryConfig());

                // quit and cleanup the old NATTypeDiscoveryClient
                if (m_natTypeDiscClient)
                {
                    m_natTypeDiscClient->quit();
                    if (m_natTypeDiscClient)
                    {
                        m_natTypeDiscClient->~bdNATTypeDiscoveryClient();
                        delete m_natTypeDiscClient;
                    }
                    m_natTypeDiscClient = NULL;
                }
                // init the new NATTypeDiscoveryClient with the new IP
                m_natTypeDiscClient = new bdNATTypeDiscoveryClient();
                bdBool startNATDiscoveryResult = m_natTypeDiscClient->init(m_params.m_socket, &addr, bdNATTypeDiscoveryConfig());
                if (!startIPDiscoveryResult || !startNATDiscoveryResult)
                {
                    bdLogWarn("bdNet/net", "Failed to start IP discovery");
                    m_status = BD_NET_ONLINE_FAILED;
                }

            }
            else
            {
                for (bdUInt i = 0; i < m_params.m_natTravHosts.getSize(); ++i)
                {
                    bdLogWarn("bdNet/net", "Failed to do IP and NAT discovery using %s", m_params.m_natTravHosts[i]->getBuffer());
                }
                m_status = BD_NET_ONLINE_FAILED;
            }
        }
    }
}

void bdNetImpl::stop()
{
    if (m_UPnP.getState())
    {
        m_UPnP.startShutdown(bdUPnPDevice::BD_UPNP_DEVICE_SHUTDOWN_ELEGANT);
    }
    m_connectionStore.startShutdown(bdConnectionStore::BD_CONNECTION_STORE_SHUTDOWN_ELEGANT);
    bdStopwatch timer;
    timer.start();
    bdBool waiting = true;
    while (waiting)
    {
        if (m_connectionStore.getStatus())
        {
            waiting = true;
            receiveAll();
            dispatchAll();
            sendAll();
        }
        if (m_UPnP.getState())
        {
            waiting = true;
            m_UPnP.pump();
        }
        if (timer.getElapsedTimeInSeconds() > 1.0f)
        {
            waiting = false;
            m_connectionStore.startShutdown(bdConnectionStore::BD_CONNECTION_STORE_SHUTDOWN_IMMEDIATE);
            m_UPnP.startShutdown(bdUPnPDevice::BD_UPNP_DEVICE_SHUTDOWN_IMMEDIATE);
        }
        if (waiting)
        {
            bdPlatformTiming::sleep(20);
        }
    }
    if (m_socketRouter)
    {
        m_socketRouter->quit();
        if (m_socketRouter)
        {
            m_socketRouter->~bdSocketRouter();
        }
        m_socketRouter = NULL;
    }
    if (m_params.m_socket)
    {
        if (!m_params.m_socket->close())
        {
            bdLogWarn("bdNet/net", "Game socket failed to close with an error");
        }
        if (m_params.m_socket)
        {
            m_params.m_socket->~bdSocket();
        }
        m_params.m_socket = NULL;
    }
    if (m_ipDiscClient)
    {
        m_ipDiscClient->quit();
        if (m_ipDiscClient)
        {
            m_ipDiscClient->~bdIPDiscoveryClient();
            delete m_ipDiscClient;
        }
        m_ipDiscClient = NULL;
    }
    if (m_natTypeDiscClient)
    {
        m_natTypeDiscClient->quit();
        if (m_natTypeDiscClient)
        {
            m_natTypeDiscClient->~bdNATTypeDiscoveryClient();
            delete m_natTypeDiscClient;
        }
    }
    if (m_getHostByName)
    {
        m_getHostByName->quit();
        if (m_getHostByName)
        {
            m_getHostByName->~bdGetHostByName();
        }
        m_getHostByName = NULL;
    }
    m_status = BD_NET_STOPPED;
    m_currentNatTravAddrIndex = 0;
    m_currentNatTravHostIndex = 0;
    m_params.m_localAddresses.clear();
    m_params.m_natTravHosts.clear();
    m_natTravAddrs.clear();
}

bdBool bdNetImpl::receiveAll()
{
    if (!m_connectionStore.receiveAll())
    {
        bdLogWarn("bdNet/net", "Failed to receive data from all connections in the connection store");
        return false;
    }
    return true;
}

void bdNetImpl::dispatchAll()
{
    m_connectionStore.dispatchAll();
}

bdBool bdNetImpl::sendAll()
{
    if (!m_connectionStore.flushAll())
    {
        bdLogWarn("bdNet/net", "Failed to flush all connections in the connection store");
        return false;
    }
    return true;
}

bdBool bdNetImpl::getBindAddr(bdAddr* addr)
{
    if (!m_params.m_useAnyIP)
    {
        if (m_params.m_localAddresses.getSize() > 1)
        {
            bdLogWarn("bdNet/net", "When binding to a specific IP, entries in m_localAddresses other than the zeroth are ignored");
        }
        if (!m_params.m_localAddresses.getSize())
        {
            bdLogError("bdNet/net", "When binding to a specific IP, m_localAddresses should not be empty");
            return false;
        }
        addr = &bdAddr(m_params.m_localAddresses[0], m_params.m_gamePort);
        return true;
    }
    addr = &bdAddr(&bdInetAddr::Any(), m_params.m_gamePort);
    return true;
}

const bdNetStartParams* bdNetImpl::getParams() const
{
    return &m_params;
}

bdNetImpl::bdNetStatus bdNetImpl::getStatus() const
{
    return m_status;
}

bdSocketRouter* bdNetImpl::getSocketRouter() const
{
    return m_socketRouter;
}

const bdConnectionStore* bdNetImpl::getConnectionStore() const
{
    return &m_connectionStore;
}

bdCommonAddrRef bdNetImpl::getLocalCommonAddr() const
{
    bdCommonAddrRef local((bdCommonAddr*)NULL);
    if (m_socketRouter)
    {
        local = m_socketRouter->getLocalCommonAddr();
    }
    return local;
}

bdString bdNetImpl::getStatusAsString() const
{
    switch (m_status)
    {
    case bdNetImpl::BD_NET_PARAMS_CONFIG_ERROR:
        return bdString("PARAMS_CONFIG_ERROR");
    case bdNetImpl::BD_NET_BIND_FAILED:
        return bdString("BIND_ERROR");
    case bdNetImpl::BD_NET_ONLINE_FAILED:
        return bdString("ONLINE_FAILED");
    case bdNetImpl::BD_NET_INIT_FAILED:
        return bdString("INIT_FAILED");
    case bdNetImpl::BD_NET_STOPPED:
        return bdString("STOPPED");
    case bdNetImpl::BD_NET_PENDING:
        return bdString("PENDING");
    case bdNetImpl::BD_NET_DONE:
        return bdString("DONE");
    default:
        return bdString("UNKNOWN_STATUS");
    }
}

bdUPnPDevice::bdUPnPPortStatus bdNetImpl::getUPnPStatus() const
{
    return m_UPnP.getPortStatus();
}
