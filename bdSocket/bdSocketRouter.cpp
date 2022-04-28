// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocket.h"

void bdSocketRouter::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdSocketRouter::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdSocketRouter::bdSocketRouter() 
    : bdNATTravListener(), bdSecurityKeyMapListener(), m_socket(NULL), m_interceptors(0), m_status(BD_SOCKET_ROUTER_UNINITIALIZED), m_config(), 
    m_localCommonAddr(), m_addrMap(), m_keyStore(), m_qosProber(), m_natTrav(), m_ECCKey(), m_qosBandwidth(), m_dtls(4u, 0.75), m_endpointToAddrMap(4u, 0.75)
{
}

bdSocketRouter::~bdSocketRouter()
{
    bdHashMap<bdEndpoint,bdDTLSAssociation*,bdEndpointHashingClass>::Iterator it;

    m_keyStore.unregisterListener();
    for (it = m_dtls.getIterator(); it; m_dtls.next(&it))
    {
        bdDTLSAssociation* value = *m_dtls.getValue(it);
        if (value)
        {
            value->~bdDTLSAssociation();
        }
    }

}

bdBool bdSocketRouter::init(bdSocket* socket, bdCommonAddrRef localCommonAddr, bdSocketRouterConfig* config)
{
    bdBool ok;

    if (m_status)
    {
        bdLogWarn("bdSocket/bdSocketRouter", "init() called multiple times.");
        return false;
    }
    if (!socket)
    {
        bdLogError("bdSocket/bdSocketRouter", "A valid socket pointer is required by this class");
        return false;
    }
    if (localCommonAddr.isNull())
    {
        bdLogError("bdSocket/bdSocketRouter", "A valid local common addr ref is required by this class");
        return false;
    }
    m_socket = socket;
    m_localCommonAddr = localCommonAddr;
    if (m_ECCKey.getStatus() == bdECCKey::BD_ECC_KEY_UNINITIALIZED)
    {
        ok = m_ECCKey.init();
    }
    if (ok)
    {
        ok = m_natTrav.init(socket, &m_qosBandwidth, bdCommonAddrRef(&m_localCommonAddr));
    }
    if (ok)
    {
        ok = m_qosProber.init(socket, &m_natTrav, &m_qosBandwidth);
    }

    if (!ok)
    {
        bdLogError("bdSocket/bdSocketRouter", "Socket Router initialization failed.");
        m_status = BD_SOCKET_ROUTER_ERROR;
        return false;
    }
    m_status = BD_SOCKET_ROUTER_INITIALIZED;
    registerInterceptor(&m_natTrav);
    registerInterceptor(&m_qosProber);
    m_keyStore.registerListener(this);
    m_config = *config;
    m_config.sanityCheckConfig();
    return true;
}

void bdSocketRouter::pump()
{
    bdHashMap<bdEndpoint, bdDTLSAssociation*, bdEndpointHashingClass>::Iterator it;
    bdEndpoint* endpoint;
    bdDTLSAssociation* conn;
    bdNChar8 addrString[22];
    bdNChar8 secIDString[18];
    bdDTLSAssociationStatus assoStatus;

    m_natTrav.pump();
    m_qosProber.pump();
    bdQueue<bdEndpoint> toRemove;
    for (it = m_dtls.getIterator(); it; m_dtls.next(&it))
    {
        endpoint = m_dtls.getKey(it);
        conn = *m_dtls.getValue(it);
        conn->pump();
        assoStatus = static_cast<bdDTLSAssociationStatus>(conn->getStatus());
        if (assoStatus == BD_SOCKET_LOST)
        {
            bdAddrHandleRef addrHandle(&conn->getAddrHandle());
            bdAddr(addrHandle->getRealAddr()).toString(addrString, sizeof(addrString));
            bdSecurityInfo::toString(endpoint->getSecID(), secIDString, sizeof(secIDString));
            bdLogInfo("bdSocket/bdSocketRouter", "DTLS closed. Removing address %s on security ID %s.", addrString, secIDString);
            bdAddrHandle::bdAddrHandleStatus status = bdAddrHandle::BD_ADDR_UNRESOLVED;
            addrHandle->setStatus(&status);
            toRemove.enqueue(endpoint);
            m_addrMap.unregisterAddr(&addrHandle);
        }
        else if ((assoStatus - 1) >= 2 && assoStatus == BD_SOCKET_IDLE)
        {
            bdLogWarn("bdSocket/bdSocketRouter", "Invalid state!");
        }
    }
    m_dtls.releaseIterator(NULL);
    while (!toRemove.isEmpty())
    {
        it = m_dtls.getIterator(toRemove.peek());
        if (!it)
        {
            bdLogWarn("bdSocket/bdSocketRouter", "Couldn't find dlts connection in map.");
            break;
        }
        conn = *m_dtls.getValue(it);
        if (conn)
        {
            delete conn;
        }
        m_dtls.remove(&it);
        toRemove.dequeue();
        m_dtls.releaseIterator(it);
    }
}

bdBool bdSocketRouter::quit()
{
    bdHashMap<bdEndpoint, bdDTLSAssociation*, bdEndpointHashingClass>::Iterator it;
    bdDTLSAssociation* conn;
    bdBool safeQuit;

    if (!m_status)
    {
        bdLogWarn("bdSocket/bdSocketRouter", "quit() called multiple times.");
        return false;
    }
    m_keyStore.unregisterListener();
    for (it = m_dtls.getIterator(); it; m_dtls.next(&it))
    {
        conn = *m_dtls.getValue(it);
        if (conn)
        {
            delete conn;
        }
    }
    m_dtls.clear();
    m_socket = NULL;
    safeQuit = m_natTrav.quit();
    safeQuit = safeQuit == m_qosProber.quit();
    unregisterInterceptor(&m_natTrav);
    unregisterInterceptor(&m_qosProber);
    m_interceptors.clear();
    m_endpointToAddrMap.clear();
    m_keyStore.clear();
    m_config.reset();
    m_status = BD_SOCKET_ROUTER_UNINITIALIZED;
    return safeQuit;
}

bdBool bdSocketRouter::connect(bdAddrHandleRef* addrHandle)
{
    bdAddrHandle::bdAddrHandleStatus status;
    bdBool added;
    bdNChar8 secIDString[18];
    bdNChar8 commonAddrInfo[1024];

    if (m_status != BD_SOCKET_ROUTER_INITIALIZED)
    {
        bdLogWarn("bdSocket/bdSocketRouter", "Cannot connect before class has been initialized");
        return false;
    }
    if (addrHandle->isNull())
    {
        bdLogWarn("bdSocket/bdSocketRouter", "Cannot connect using null addr handle ref");
        return false;
    }
    bdCommonAddrRef remote((*addrHandle)->m_endpoint.getCommonAddr());
    bdLogInfo("bdSocket/bdSocketRouter", "\tConnecting to address.");
    bdCommonAddrInfo::getInfo(bdCommonAddrRef(&remote), commonAddrInfo, sizeof(commonAddrInfo));
    bdLogInfo("bdSocket/bdSocketRouter", commonAddrInfo);
    bdSecurityInfo::toString((*addrHandle)->m_endpoint.getSecID(), secIDString, sizeof(secIDString));
    bdLogInfo("bdSocket/bdSocketRouter", "Using security ID: %s", secIDString);
    if (!m_keyStore.contains((*addrHandle)->m_endpoint.getSecID()))
    {
        bdLogWarn("bdSocket/bdSocketRouter", "Security ID is not registered. ");
    }

    switch ((*addrHandle)->getStatus())
    {
    case bdAddrHandle::BD_ADDR_RESOLVED:
        bdLogInfo("bdSocket/bdSocketRouter", "Address already resolved. Nothing to do.");
        break;
    case bdAddrHandle::BD_ADDR_ERROR:
        bdLogInfo("bdSocket/bdSocketRouter", "Address resolution failed. Trying again.");
        break;
    case bdAddrHandle::BD_ADDR_UNRESOLVED:
        bdLogInfo("bdSocket/bdSocketRouter", "Address has been unregistered. Starting again.");
        break;
    case bdAddrHandle::BD_ADDR_NOT_RESOLVED:
    {
        bdLogInfo("bdSocket/bdSocketRouter", "Address not resolved. Sending to NAT Trav.");
        bdEndpoint endpoint(&bdCommonAddrRef(&remote), (*addrHandle)->m_endpoint.getSecID());
        added = m_endpointToAddrMap.put(&endpoint, addrHandle);
        if (!added)
        {
            bdLogWarn("bdSocket/bdSocketRouter", "Remote addr already exists in map");
            bdLogInfo("bdSocket/bdSocketRouter", "Cancelling existing NAT Trav if any....");
            m_natTrav.cancelConnect(bdCommonAddrRef(&remote));
            bdLogInfo("bdSocket/bdSocketRouter", "Removing addr from map .... and re-adding");
            m_endpointToAddrMap.remove(&endpoint);
            added = m_endpointToAddrMap.put(&endpoint, addrHandle);
        }
        if (!added)
        {
            bdLogError("bdSocket/bdSocketRouter", "Repeated failure to add addr to map");
            return false;
        }
        return m_natTrav.connect(bdCommonAddrRef(&remote), this, false);
    }
    }
    return false;
}

bdBool bdSocketRouter::disconnect(bdAddrHandleRef* addrHandle)
{
    bdDTLSAssociation* conn;
    bdUInt numFound = 0;
    bdHashMap<bdEndpoint, bdDTLSAssociation*, bdEndpointHashingClass>::Iterator it;

    if (m_status != BD_SOCKET_ROUTER_INITIALIZED)
    {
        bdLogWarn("bdSocket/bdSocketRouter", "Cannot disconnect unless class is initialized");
        return false;
    }
    if (addrHandle->isNull())
    {
        bdLogWarn("bdSocket/bdSocketRouter", "Cannot disconnect using null addr handle ref");
        return false;
    }
    for (it = m_dtls.getIterator(); it; m_dtls.next(&it))
    {
        conn = *m_dtls.getValue(it);
        if (bdAddrHandleRef(&conn->getAddrHandle()) == addrHandle)
        {
            delete conn;
            ++numFound;
            m_dtls.remove(&it);
        }
    }
    m_dtls.releaseIterator(NULL);
    if (numFound > 1)
    {
        bdLogError("bdSocket/bdSocketRouter", "More than one DTLS connection found for an addrHandle");
    }
    return m_addrMap.unregisterAddr(addrHandle);
}

bdInt bdSocketRouter::sendTo(const bdAddrHandleRef addrHandle, void* data, const bdUInt length)
{
    bdDTLSAssociation* dtls;
    bdNChar8 commonAddrInfo[1024];

    if (m_status != BD_SOCKET_ROUTER_INITIALIZED)
    {
        bdLogWarn("bdSocket/bdSocketRouter", "Cannot send data before class has been initialized");
        return -1;
    }
    if (addrHandle.isNull())
    {
        bdLogWarn("bdSocket/bdSocketRouter", "Cannot send data to null addr handle ref");
        return -1;
    }
    bdEndpoint endpoint(&bdCommonAddrRef(addrHandle->m_endpoint.getCommonAddr()), addrHandle->m_endpoint.getSecID());
    m_dtls.get(&endpoint, &dtls);
    if (!dtls)
    {
        bdLogError("bdSocket/bdSocketRouter", "Not established! Call connect() first.");
        bdCommonAddrInfo::getBriefInfo(&bdAddrHandleRef(addrHandle)->m_endpoint.getCommonAddr(), commonAddrInfo, sizeof(commonAddrInfo));
        bdLogInfo("bdSocket/bdSocketRouter", commonAddrInfo);
        return -1;
    }
    return dtls->sendTo(addrHandle->getRealAddr(), data, length, addrHandle->m_endpoint.getSecID());
}

bdInt bdSocketRouter::receiveFrom(bdAddrHandleRef* addrHandle, void* data, const bdUInt size)
{
    bdInt val;

    if (m_status != BD_SOCKET_ROUTER_INITIALIZED)
    {
        bdLogWarn("bdSocket/bdSocketRouter", "Cannot receive data before class has been initialized");
        return -1;
    }
    bdBool receiving = true;
    bdUInt resetCount = 0;
    bdAddr realAddr;
    while (receiving)
    {
        val = m_socket->receiveFrom(&realAddr, data, size);
        if (val > 0)
        {
            val = processPacket(addrHandle, &realAddr, data, size, val, &receiving);
            continue;
        }
        if (val < 0)
        {
            processError(&realAddr, val, &resetCount, &receiving);
            continue;
        }
        bdLogError("bdSocket/bdSocketRouter", "socket receiveFrom returned 0 ");
        receiving = false;
    }
    return val;
}

bdInt bdSocketRouter::processPacket(bdAddrHandleRef* addrHandle, bdAddr* realAddr, void* data, const bdUInt size, const bdUInt bytesTransferred, bdBool* receiving)
{
    bdUInt tmpUInt;
    bdNChar8 addrString[22];
    bdNChar8 secIDString[18];

    bdUInt recievedBytes = bytesTransferred;
    bdUByte8 type = *(bdUByte8*)data;
    bdBool createNewDTLS = false;
    switch (type)
    {

    case 1:
    {
        bdDTLSInit init;
        tmpUInt = 0;
        if (!init.deserialize(data, recievedBytes, 0, &tmpUInt))
        {
            break;
        }
        bdSecurityID initSecID;
        init.getSecID(&initSecID);
        if (tryToFindConnection(data, &size, &initSecID, realAddr, addrHandle, (bdInt*)&recievedBytes, true))
        {
            break;
        }
        bdDTLSAssociation serverDTLS(
            m_socket,
            &m_keyStore,
            &m_ECCKey,
            realAddr,
            bdAddrHandleRef(addrHandle),
            bdCommonAddrRef(&m_localCommonAddr),
            &m_addrMap,
            m_config.m_DTLSAssociationReceiveTimeout
        );
        recievedBytes = serverDTLS.receiveFrom(realAddr, data, recievedBytes, addrHandle, data, size);
        createNewDTLS = serverDTLS.getStatus() == BD_SOCKET_CONNECTED;
        break;
    }
    case 2:
    {
        bdDTLSInitAck initAck;
        tmpUInt = 0;
        if (!initAck.deserialize(data, recievedBytes, 0, &tmpUInt))
        {
            break;
        }
        bdSecurityID initAckSecID;
        initAck.getSecID(&initAckSecID);
        tryDecryptPacket(data, &size, &initAckSecID, realAddr, addrHandle, (bdInt*)&recievedBytes, true);
        break;
    }
    case 3:
    {
        bdDTLSCookieEcho cookieEcho;
        tmpUInt = 0;
        if (!cookieEcho.deserialize(data, recievedBytes, 0, &tmpUInt))
        {
            break;
        }
        bdSecurityID cookieEchoSecID;
        bdMemcpy(&cookieEchoSecID, cookieEcho.getSecID(), sizeof(cookieEchoSecID));
        if (tryToFindConnection(data, &size, &cookieEchoSecID, realAddr, addrHandle, (bdInt*)&recievedBytes, true))
        {
            break;
        }
        bdDTLSAssociation serverDTLS(
            m_socket,
            &m_keyStore,
            &m_ECCKey,
            realAddr,
            bdAddrHandleRef(addrHandle),
            bdCommonAddrRef(&m_localCommonAddr),
            &m_addrMap,
            m_config.m_DTLSAssociationReceiveTimeout
        );
        recievedBytes = serverDTLS.receiveFrom(realAddr, data, recievedBytes, addrHandle, data, size);
        break;
    }
    case 4:
    {
        bdDTLSCookieAck cookieAck;
        tmpUInt = 0;
        if (!cookieAck.deserialize(data, recievedBytes, 0, &tmpUInt))
        {
            break;
        }
        bdSecurityID cookieAckSecID;
        cookieAck.getSecID(&cookieAckSecID);
        tryDecryptPacket(data, &size, &cookieAckSecID, realAddr, addrHandle, (bdInt*)&recievedBytes, true);
        break;
    }
    case 5:
    {
        bdDTLSError error;
        tmpUInt = 0;
        if (!error.deserialize(data, recievedBytes, 0, &tmpUInt))
        {
            break;
        }
        bdSecurityID errorSecID;
        error.getSecID(&errorSecID);
        if (tryToFindConnection(data, &size, &errorSecID, realAddr, addrHandle, (bdInt*)&recievedBytes, true))
        {
            break;
        }
        bdDTLSAssociation errorDTLS(
            m_socket,
            &m_keyStore,
            &m_ECCKey,
            realAddr,
            bdAddrHandleRef(addrHandle),
            bdCommonAddrRef(&m_localCommonAddr),
            &m_addrMap,
            m_config.m_DTLSAssociationReceiveTimeout
        );
        recievedBytes = errorDTLS.receiveFrom(realAddr, data, recievedBytes, addrHandle, data, size);
        createNewDTLS = errorDTLS.getStatus() == BD_SOCKET_CONNECTED;
        break;
    }
    case 6:
    {
        bdSecurityID secID;
        *receiving = !tryDecryptPacket(data, &size, &secID, realAddr, addrHandle, (bdInt*)&recievedBytes, true);
        break;
    }
    default:
    {
        bdBool accepted = false;
        for (bdUInt i = 0; !accepted; ++i)
        {
            if (i >= m_interceptors.getSize())
            {
                break;
            }
            accepted = (*m_interceptors[i])->acceptPacket(m_socket, bdAddr(realAddr), data, recievedBytes, type);
        }
        break;
    }
    }
    if (!createNewDTLS)
    {
        return recievedBytes;
    }
    bdDTLSAssociation* dtlsSocket = new bdDTLSAssociation(
        m_socket,
        &m_keyStore,
        &m_ECCKey,
        realAddr,
        bdAddrHandleRef(addrHandle),
        bdCommonAddrRef(&m_localCommonAddr),
        &m_addrMap,
        m_config.m_DTLSAssociationReceiveTimeout
    );
    recievedBytes = dtlsSocket->receiveFrom(realAddr, data, recievedBytes, addrHandle, data, size);
    realAddr->toString(addrString, sizeof(addrString));
    bdLogInfo("bdSocket/bdSocketRouter", "New incoming DTLS connection from %s.", addrString);
    bdEndpoint endpoint(&bdCommonAddrRef((*addrHandle)->m_endpoint.getCommonAddr()), (*addrHandle)->m_endpoint.getSecID());
    if (!m_dtls.put(&endpoint, &dtlsSocket))
    {
        bdCommonAddrInfo::getInfo(bdCommonAddrRef((*addrHandle)->m_endpoint.getCommonAddr()), addrString, sizeof(addrString));
        bdSecurityInfo::toString(endpoint.getSecID(), secIDString, sizeof(secIDString));
        bdLogError("bdSocket/bdSocketRouter", "New DTLS with secID %s NOT added to the list.", secIDString);
        bdLogError("bdSocket/bdSocketRouter", "addrString %s", addrString);
        if (dtlsSocket)
        {
            delete dtlsSocket;
        }
    }
    if ((recievedBytes & 0x80000000) == 0)
    {
        *receiving = false;
    }
    return recievedBytes;
}

void bdSocketRouter::processError(bdAddr* realAddr, bdInt val, bdUInt* resetCount, bdBool* receiving)
{
}

bdBool bdSocketRouter::tryToFindConnection(void* data, const bdUInt* size, const bdSecurityID* secID, bdAddr* realAddr, bdAddrHandleRef* addrHandle, bdInt* val, bdBool checkSecID)
{
    return bdBool();
}

bdBool bdSocketRouter::tryDecryptPacket(void* data, const bdUInt* size, const bdSecurityID* secID, bdAddr* realAddr, bdAddrHandleRef* addrHandle, bdInt* val, bdBool checkSecID)
{
    return bdBool();
}

void bdSocketRouter::onNATAddrDiscovery(bdCommonAddrRef remote, const bdAddr* realAddr)
{
}

void bdSocketRouter::onNATAddrDiscoveryFailed(bdCommonAddrRef remote)
{
}

bdBool bdSocketRouter::setupIntroducers(const bdArray<bdAddr>* introducers)
{
    return bdBool();
}

void bdSocketRouter::onSecurityKeyRemove(const bdSecurityID* id)
{
}

void bdSocketRouter::registerInterceptor(bdPacketInterceptor* const interceptor)
{
}

void bdSocketRouter::unregisterInterceptor(bdPacketInterceptor* const interceptor)
{
}

const bdInt bdSocketRouter::getStatus(bdCommonAddrRef localCommonAddr) const
{
    return bdInt();
}

const bdQoSProbe* bdSocketRouter::getQoSProber() const
{
    return NULL;
}

const bdAddressMap* bdSocketRouter::getAddressMap() const
{
    return NULL;
}

const bdSecurityKeyMap* bdSocketRouter::getKeyMap() const
{
    return NULL;
}

const bdCommonAddrRef bdSocketRouter::getLocalCommonAddr() const
{
    return bdCommonAddrRef();
}
