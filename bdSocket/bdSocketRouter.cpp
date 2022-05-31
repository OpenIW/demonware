// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocket.h"

const bdNChar8* LogChannel = "bdSocket/bdSocketRouter";

void bdSocketRouter::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdSocketRouter::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdSocketRouter::bdSocketRouter() 
    : bdNATTravListener(), bdSecurityKeyMapListener(), m_socket(NULL), m_interceptors(0u), m_status(BD_SOCKET_ROUTER_UNINITIALIZED), m_config(), 
    m_localCommonAddr(), m_addrMap(), m_keyStore(), m_qosProber(), m_natTrav(), m_ECCKey(), m_qosBandwidth(), m_dtls(4u, 0.75), m_endpointToAddrMap(4u, 0.75)
{
}

bdSocketRouter::~bdSocketRouter()
{
    bdHashMap<bdEndpoint,bdDTLSAssociation*,bdEndpointHashingClass>::Iterator it;

    m_keyStore.unregisterListener();
    for (it = m_dtls.getIterator(); it; m_dtls.next(it))
    {
        bdDTLSAssociation* value = m_dtls.getValue(it);
        if (value)
        {
            value->~bdDTLSAssociation();
        }
    }

}

bdBool bdSocketRouter::init(bdSocket* socket, bdCommonAddrRef localCommonAddr, bdSocketRouterConfig& config)
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
        ok = m_natTrav.init(socket, &m_qosBandwidth, bdCommonAddrRef(m_localCommonAddr));
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
    m_config = config;
    m_config.sanityCheckConfig();
    return true;
}

void bdSocketRouter::pump()
{
    bdHashMap<bdEndpoint, bdDTLSAssociation*, bdEndpointHashingClass>::Iterator it;
    bdDTLSAssociation* conn;
    bdNChar8 addrString[22];
    bdNChar8 secIDString[18];
    bdDTLSAssociationStatus assoStatus;

    m_natTrav.pump();
    m_qosProber.pump();
    bdQueue<bdEndpoint> toRemove;
    for (it = m_dtls.getIterator(); it; m_dtls.next(it))
    {
        bdEndpoint endpoint = m_dtls.getKey(it);
        conn = m_dtls.getValue(it);
        conn->pump();
        assoStatus = static_cast<bdDTLSAssociationStatus>(conn->getStatus());
        if (assoStatus == BD_SOCKET_LOST)
        {
            bdAddrHandleRef addrHandle(conn->getAddrHandle());
            bdAddr(addrHandle->getRealAddr()).toString(addrString, sizeof(addrString));
            bdSecurityInfo::toString(endpoint.getSecID(), secIDString, sizeof(secIDString));
            bdLogInfo("bdSocket/bdSocketRouter", "DTLS closed. Removing address %s on security ID %s.", addrString, secIDString);
            bdAddrHandle::bdAddrHandleStatus status = bdAddrHandle::BD_ADDR_UNRESOLVED;
            addrHandle->setStatus(status);
            toRemove.enqueue(endpoint);
            m_addrMap.unregisterAddr(addrHandle);
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
        conn = m_dtls.getValue(it);
        if (conn)
        {
            delete conn;
        }
        m_dtls.remove(it);
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
    for (it = m_dtls.getIterator(); it; m_dtls.next(it))
    {
        conn = m_dtls.getValue(it);
        if (conn)
        {
            conn->~bdDTLSAssociation();
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

bdBool bdSocketRouter::connect(bdAddrHandleRef& addrHandle)
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
    if (addrHandle.isNull())
    {
        bdLogWarn("bdSocket/bdSocketRouter", "Cannot connect using null addr handle ref");
        return false;
    }
    bdCommonAddrRef remote(addrHandle->m_endpoint.getCommonAddr());
    bdLogInfo("bdSocket/bdSocketRouter", "\tConnecting to address.");
    bdCommonAddrInfo::getInfo(bdCommonAddrRef(remote), commonAddrInfo, sizeof(commonAddrInfo));
    bdLogInfo("bdSocket/bdSocketRouter", commonAddrInfo);
    bdSecurityInfo::toString(addrHandle->m_endpoint.getSecID(), secIDString, sizeof(secIDString));
    bdLogInfo("bdSocket/bdSocketRouter", "Using security ID: %s", secIDString);
    if (!m_keyStore.contains(addrHandle->m_endpoint.getSecID()))
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
        bdEndpoint endpoint(bdCommonAddrRef(remote), addrHandle->m_endpoint.getSecID());
        added = m_endpointToAddrMap.put(endpoint, addrHandle);
        if (!added)
        {
            bdLogWarn("bdSocket/bdSocketRouter", "Remote addr already exists in map");
            bdLogInfo("bdSocket/bdSocketRouter", "Cancelling existing NAT Trav if any....");
            m_natTrav.cancelConnect(bdCommonAddrRef(remote));
            bdLogInfo("bdSocket/bdSocketRouter", "Removing addr from map .... and re-adding");
            m_endpointToAddrMap.remove(endpoint);
            added = m_endpointToAddrMap.put(endpoint, addrHandle);
        }
        if (!added)
        {
            bdLogError("bdSocket/bdSocketRouter", "Repeated failure to add addr to map");
            return false;
        }
        return m_natTrav.connect(bdCommonAddrRef(remote), this, false);
    }
    }
    return false;
}

bdBool bdSocketRouter::disconnect(bdAddrHandleRef& addrHandle)
{
    bdDTLSAssociation* conn;
    bdUInt numFound = 0;
    bdHashMap<bdEndpoint, bdDTLSAssociation*, bdEndpointHashingClass>::Iterator it;

    if (m_status != BD_SOCKET_ROUTER_INITIALIZED)
    {
        bdLogWarn("bdSocket/bdSocketRouter", "Cannot disconnect unless class is initialized");
        return false;
    }
    if (addrHandle.isNull())
    {
        bdLogWarn("bdSocket/bdSocketRouter", "Cannot disconnect using null addr handle ref");
        return false;
    }
    for (it = m_dtls.getIterator(); it; m_dtls.next(it))
    {
        conn = m_dtls.getValue(it);
        if (bdAddrHandleRef(conn->getAddrHandle()) == addrHandle)
        {
            delete conn;
            ++numFound;
            m_dtls.remove(it);
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
    bdEndpoint endpoint(bdCommonAddrRef(addrHandle->m_endpoint.getCommonAddr()), addrHandle->m_endpoint.getSecID());
    m_dtls.get(endpoint, dtls);
    if (!dtls)
    {
        bdLogError("bdSocket/bdSocketRouter", "Not established! Call connect() first.");
        bdCommonAddrInfo::getBriefInfo(bdAddrHandleRef(addrHandle)->m_endpoint.getCommonAddr(), commonAddrInfo, sizeof(commonAddrInfo));
        bdLogInfo("bdSocket/bdSocketRouter", commonAddrInfo);
        return -1;
    }
    return dtls->sendTo(addrHandle->getRealAddr(), data, length, addrHandle->m_endpoint.getSecID());
}

bdInt bdSocketRouter::receiveFrom(bdAddrHandleRef& addrHandle, void* data, const bdUInt size)
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
        val = m_socket->receiveFrom(realAddr, data, size);
        if (val > 0)
        {
            val = processPacket(addrHandle, realAddr, data, size, val, receiving);
            continue;
        }
        if (val < 0)
        {
            processError(realAddr, val, resetCount, receiving);
            continue;
        }
        bdLogError("bdSocket/bdSocketRouter", "socket receiveFrom returned 0 ");
        receiving = false;
    }
    return val;
}

bdInt bdSocketRouter::processPacket(bdAddrHandleRef& addrHandle, bdAddr& realAddr, void* data, const bdUInt size, const bdUInt bytesTransferred, bdBool& receiving)
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
        if (!init.deserialize(data, recievedBytes, 0, tmpUInt))
        {
            break;
        }
        bdSecurityID initSecID;
        init.getSecID(initSecID);
        if (tryToFindConnection(data, size, initSecID, realAddr, addrHandle, (bdInt&)recievedBytes, true))
        {
            break;
        }
        bdDTLSAssociation serverDTLS(
            m_socket,
            &m_keyStore,
            &m_ECCKey,
            realAddr,
            bdAddrHandleRef(addrHandle),
            bdCommonAddrRef(m_localCommonAddr),
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
        if (!initAck.deserialize(data, recievedBytes, 0, tmpUInt))
        {
            break;
        }
        bdSecurityID initAckSecID;
        initAck.getSecID(initAckSecID);
        tryDecryptPacket(data, size, initAckSecID, realAddr, addrHandle, (bdInt&)recievedBytes, true);
        break;
    }
    case 3:
    {
        bdDTLSCookieEcho cookieEcho;
        tmpUInt = 0;
        if (!cookieEcho.deserialize(data, recievedBytes, 0, tmpUInt))
        {
            break;
        }
        bdSecurityID cookieEchoSecID;
        bdMemcpy(cookieEchoSecID.ab, cookieEcho.getSecID(), sizeof(cookieEchoSecID));
        if (tryToFindConnection(data, size, cookieEchoSecID, realAddr, addrHandle, (bdInt&)recievedBytes, true))
        {
            break;
        }
        bdDTLSAssociation serverDTLS(
            m_socket,
            &m_keyStore,
            &m_ECCKey,
            realAddr,
            bdAddrHandleRef(addrHandle),
            bdCommonAddrRef(m_localCommonAddr),
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
        if (!cookieAck.deserialize(data, recievedBytes, 0, tmpUInt))
        {
            break;
        }
        bdSecurityID cookieAckSecID;
        cookieAck.getSecID(cookieAckSecID);
        tryDecryptPacket(data, size, cookieAckSecID, realAddr, addrHandle, (bdInt&)recievedBytes, true);
        break;
    }
    case 5:
    {
        bdDTLSError error;
        tmpUInt = 0;
        if (!error.deserialize(data, recievedBytes, 0, tmpUInt))
        {
            break;
        }
        bdSecurityID errorSecID;
        error.getSecID(errorSecID);
        if (tryToFindConnection(data, size, errorSecID, realAddr, addrHandle, (bdInt&)recievedBytes, true))
        {
            break;
        }
        bdDTLSAssociation errorDTLS(
            m_socket,
            &m_keyStore,
            &m_ECCKey,
            realAddr,
            bdAddrHandleRef(addrHandle),
            bdCommonAddrRef(m_localCommonAddr),
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
        receiving = !tryDecryptPacket(data, size, secID, realAddr, addrHandle, (bdInt&)recievedBytes, true);
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
            accepted = m_interceptors[i]->acceptPacket(m_socket, bdAddr(realAddr), data, recievedBytes, type);
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
        bdCommonAddrRef(m_localCommonAddr),
        &m_addrMap,
        m_config.m_DTLSAssociationReceiveTimeout
    );
    recievedBytes = dtlsSocket->receiveFrom(realAddr, data, recievedBytes, addrHandle, data, size);
    realAddr.toString(addrString, sizeof(addrString));
    bdLogInfo("bdSocket/bdSocketRouter", "New incoming DTLS connection from %s.", addrString);
    bdEndpoint endpoint(bdCommonAddrRef(addrHandle->m_endpoint.getCommonAddr()), addrHandle->m_endpoint.getSecID());
    if (!m_dtls.put(endpoint, dtlsSocket))
    {
        bdCommonAddrInfo::getInfo(bdCommonAddrRef(addrHandle->m_endpoint.getCommonAddr()), addrString, sizeof(addrString));
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
        receiving = false;
    }
    return recievedBytes;
}

void bdSocketRouter::processError(bdAddr& realAddr, bdInt val, bdUInt& resetCount, bdBool& receiving)
{
    bdNChar8 addrBuffer[100];

    receiving = false;
    switch (val)
    {
    case -13:
    case -12:
    case -11:
    case -10:
    case -9:
    case -8:
    case -7:
    case -4:
    case -3:
    case -1:
        bdLogWarn("bdSocket/bdSocketRouter", "Encountered error: %i while receving from the socket router ", val);
        break;
    case -6:
        bdLogWarn("bdSocket/bdSocketRouter", "Couldn't recieve message. Buffer too small?");
        break;
    case -5:
        if (m_config.m_ignoreConnectionReset)
        {
            realAddr.toString(addrBuffer, sizeof(addrBuffer));
            bdLogInfo("bdSocket/bdSocketRouter", "Connection reset packet received for %s. Ignoring.", addrBuffer);
        }
        else
        {
            m_addrMap.unregisterRealAddr(bdAddr(realAddr));
        }
        if (resetCount++ < m_config.m_maxConnectionResets)
        {
            receiving = true;
            break;
        }
        realAddr.toString(addrBuffer, sizeof(addrBuffer));
        bdLogWarn("bdSocket/bdSocketRouter", "Exiting receiveAll loop after receiving %u connection resets.", resetCount);
        bdLogWarn("bdSocket/bdSocketRouter", "Last reset was received for addr %s ", addrBuffer);
        break;
    case 0:
    case 1:
        bdLogError("bdSocket/bdSocketRouter", "These cannot occur");
        break;
    default:
        break;
    }
}

bdBool bdSocketRouter::tryToFindConnection(void* data, const bdUInt& size, const bdSecurityID& secID, bdAddr& realAddr, bdAddrHandleRef& addrHandle, bdInt& val, bdBool checkSecID)
{
    bdHashMap<bdEndpoint,bdDTLSAssociation*,bdEndpointHashingClass>::Iterator it;
    bdDTLSAssociation* dtls;
    bdInt returnVal;
    bdBool found = false;

    for (it = m_dtls.getIterator(); it, !found; m_dtls.next(it))
    {
        bdEndpoint key(m_dtls.getKey(it));
        dtls = m_dtls.getValue(it);
        bdAddrHandleRef dtlsAddrHandle;
        bdAddr dtlsRealAddr;
        if (dtls)
        {
            dtlsAddrHandle = bdAddrHandleRef(dtls->getAddrHandle());
        }
        if (dtlsAddrHandle.notNull())
        {
            bdMemcpy(&dtlsRealAddr, &bdAddr(dtlsAddrHandle->getRealAddr()), sizeof(dtlsRealAddr));
        }

        if (dtlsAddrHandle.notNull() && dtlsRealAddr == realAddr)
        {
            if (!checkSecID || (key.getSecID() == secID))
            {
                returnVal = dtls->receiveFrom(realAddr, data, val, addrHandle, data, size);
                if (returnVal >= 0)
                {
                    found = true;
                    val = returnVal;
                    m_dtls.releaseIterator(it);
                }
                if (returnVal == -2)
                {
                    found = true;
                    val = -2;
                    m_dtls.releaseIterator(it);
                }
            }
        }
    }
    return found;
}

bdBool bdSocketRouter::tryDecryptPacket(void* data, const bdUInt& size, const bdSecurityID& secID, bdAddr& realAddr, bdAddrHandleRef& addrHandle, bdInt& val, bdBool checkSecID)
{
    bdHashMap<bdEndpoint, bdDTLSAssociation*, bdEndpointHashingClass>::Iterator it;
    bdDTLSAssociation* dtls;
    bdInt returnVal;
    bdBool found = false;

    for (it = m_dtls.getIterator(); it, !found; m_dtls.next(it))
    {
        bdEndpoint key(m_dtls.getKey(it));
        dtls = m_dtls.getValue(it);
        bdAddrHandleRef dtlsAddrHandle;
        bdAddr dtlsRealAddr;
        if (dtls)
        {
            dtlsAddrHandle = bdAddrHandleRef(dtls->getAddrHandle());
        }
        if (dtlsAddrHandle.notNull())
        {
            bdMemcpy(&dtlsRealAddr, &bdAddr(dtlsAddrHandle->getRealAddr()), sizeof(dtlsRealAddr));
        }

        if (dtlsAddrHandle.notNull() && dtlsRealAddr == realAddr)
        {
            if (!checkSecID || (key.getSecID() == secID))
            {
                returnVal = dtls->receiveFrom(realAddr, data, val, addrHandle, data, size);
                if (returnVal >= 0)
                {
                    found = true;
                    val = returnVal;
                    m_dtls.releaseIterator(it);
                }
            }
        }
    }
    return found;
}

void bdSocketRouter::onNATAddrDiscovery(bdCommonAddrRef remote, const bdAddr& realAddr)
{
    bdHashMap<bdEndpoint,bdReference<bdAddrHandle>,bdEndpointHashingClass>::Iterator it;
    bdDTLSAssociation* conn;
    bdNChar8 addrString[22];
    bdNChar8 secIDString[18];
    bdNChar8 commonAddrInfo[1024];
    bdNChar8 commonAddrString[1024];

    bdAddrHandleRef addrHandle;
    if (m_status != BD_SOCKET_ROUTER_INITIALIZED)
    {
        bdLogWarn("bdSocket/bdSocketRouter", "Cannot react to a NAT addr discovery event before the class has been initialized");
        return;
    }
    bdBool found = false;
    for (it = m_endpointToAddrMap.getIterator(); !found, it; m_endpointToAddrMap.next(it))
    {
        bdEndpoint key(m_endpointToAddrMap.getKey(it));
        addrHandle = m_endpointToAddrMap.getValue(it);
        if (bdCommonAddrRef(key.getCommonAddr()) == remote)
        {
            continue;
        }
        found = true;
        m_endpointToAddrMap.remove(it);
        addrHandle->setRealAddr(realAddr);
        bdEndpoint endpoint(key.getCommonAddr(), addrHandle->m_endpoint.getSecID());
        if (m_dtls.get(endpoint, conn))
        {
            bdLogWarn("bdSocket/bdSocketRouter", "already connected.");
            continue;
        }
        realAddr.toString(addrString, sizeof(addrString));
        bdLogInfo("bdSocket/bdSocketRouter", "Address lookup succeeded. Creating DTLS conn to %s.", addrString);
        conn = new bdDTLSAssociation(
            m_socket,
            &m_keyStore,
            &m_ECCKey,
            realAddr,
            bdAddrHandleRef(addrHandle),
            bdCommonAddrRef(m_localCommonAddr),
            &m_addrMap,
            m_config.m_DTLSAssociationReceiveTimeout
        );
        conn->connect();
        if (!m_dtls.put(key, conn))
        {
            bdCommonAddrInfo::getInfo(endpoint.getCommonAddr(), commonAddrString, sizeof(commonAddrString));
            bdSecurityInfo::toString(endpoint.getSecID(), secIDString, sizeof(secIDString));
            bdLogError("bdSocket/bdSocketRouter", "New DTLS with secID %s NOT added to the list.", secIDString);
            bdLogError("bdSocket/bdSocketRouter", "Address: %s", commonAddrString);
            if (conn)
            {
                delete conn;
            }
        }
    }
    if (!found)
    {
        bdLogError("bdSocket/bdSocketRouter", "Address lookup failed");
        bdCommonAddrInfo::getInfo(bdCommonAddrRef(remote), commonAddrInfo, sizeof(commonAddrInfo));
        bdLogInfo("bdSocket/bdSocketRouter", commonAddrInfo);
    }
}

void bdSocketRouter::onNATAddrDiscoveryFailed(bdCommonAddrRef remote)
{
    bdNChar8 commonAddrInfo[1024];
    bdHashMap<bdEndpoint, bdReference<bdAddrHandle>, bdEndpointHashingClass>::Iterator it;
    bdDTLSAssociation* conn;
    bdBool found = false;

    bdLogInfo(LogChannel, "Address lookup failed from");
    bdLogInfo(LogChannel, "NAT Discovery Failed - Local addr:");
    bdCommonAddrInfo::getInfo(&bdCommonAddrRef(&m_localCommonAddr), commonAddrInfo, sizeof(commonAddrInfo));
    bdLogInfo(LogChannel, commonAddrInfo);
    bdLogInfo(LogChannel, "NAT Discovery Failed - Remote addr:");
    bdCommonAddrInfo::getInfo(&bdCommonAddrRef(remote), commonAddrInfo, sizeof(commonAddrInfo));
    bdLogInfo(LogChannel, commonAddrInfo);

    bdAddrHandleRef addrHandle;
    for (it = m_endpointToAddrMap.getIterator(); !found, it; m_endpointToAddrMap.next(it))
    {
        bdEndpoint key(m_endpointToAddrMap.getKey(it));
        addrHandle = m_endpointToAddrMap.getValue(it);
        if (!(*bdCommonAddrRef(key.getCommonAddr()) == *remote))
        {
            continue;
        }
        found = true;
        m_endpointToAddrMap.remove(it);
        if (addrHandle->getStatus() == bdAddrHandle::BD_ADDR_RESOLVED)
        {
            bdAddrHandle::bdAddrHandleStatus status = bdAddrHandle::BD_ADDR_ERROR;
            addrHandle->setStatus(status);
        }
    }
    if (!found)
    {
        bdLogError(LogChannel, "address lookup failed.");
    }
}

bdBool bdSocketRouter::setupIntroducers(const bdArray<bdAddr>& introducers)
{
    if (m_status == BD_SOCKET_ROUTER_INITIALIZED)
    {
        return m_natTrav.setupIntroducers(introducers);
    }
    return false;
}

void bdSocketRouter::onSecurityKeyRemove(const bdSecurityID& id)
{
    bdHashMap<bdEndpoint,bdDTLSAssociation*,bdEndpointHashingClass>::Iterator it;
    bdDTLSAssociation* conn;
    bdAddrHandle::bdAddrHandleStatus status;

    for (it = m_dtls.getIterator(); it; m_dtls.next(it))
    {
        conn = m_dtls.getValue(it);
        if (conn->getLocalSecurityId() == id)
        {
            bdAddrHandleRef addrHandle(conn->getAddrHandle());
            status = bdAddrHandle::BD_ADDR_UNRESOLVED;
            addrHandle->setStatus(status);
            if (conn)
            {
                delete conn;
            }
            m_dtls.remove(it);
            m_addrMap.unregisterAddr(addrHandle);
        }
    }
}

void bdSocketRouter::registerInterceptor(bdPacketInterceptor* const interceptor)
{
    bdUInt32 index;

    if (m_status == BD_SOCKET_ROUTER_INITIALIZED)
    {
        if (m_interceptors.findFirst(interceptor, index))
        {
            bdLogWarn("bdSocket/bdSocketRouter", "same listener registered multiple times.");
        }
        m_interceptors.pushBack(interceptor);
    }
    else
    {
        bdLogError("bdSocket/bdSocketRouter", "Cannot register interceptors until this class is initialized");
    }
}

void bdSocketRouter::unregisterInterceptor(bdPacketInterceptor* const interceptor)
{
    m_interceptors.removeAllKeepOrder(interceptor);
}

bdInt bdSocketRouter::getStatus(const bdAddrHandleRef addrHandle)
{
    bdNChar8 addrStr[22];

    bdDTLSAssociationStatus status = BD_SOCKET_IDLE;
    if (addrHandle.isNull())
    {
        status = BD_SOCKET_LOST;
        bdLogWarn(LogChannel, "Cannot get status of null addr handle ref");
        return status;
    }
    bdAddressMap::addrToString(&addrHandle, addrStr, sizeof(addrStr));
    switch (addrHandle->getStatus())
    {
    case bdAddrHandle::BD_ADDR_NOT_CONSTRUCTED:
        status = BD_SOCKET_IDLE;
        break;
    case bdAddrHandle::BD_ADDR_NOT_RESOLVED:
        status = m_natTrav.isConnectInProgress(addrHandle->m_endpoint.getCommonAddr()) ? BD_SOCKET_PENDING : BD_SOCKET_LOST;
        break;
    case bdAddrHandle::BD_ADDR_ERROR:
        bdLogInfo(LogChannel, "Address handle error. Socket lost. %s", addrStr);
        status = BD_SOCKET_LOST;
        break;
    case bdAddrHandle::BD_ADDR_RESOLVED:
    {
        bdEndpoint key(addrHandle->m_endpoint.getCommonAddr(), addrHandle->m_endpoint.getSecID());
        bdDTLSAssociation* dtlsSocket = NULL;
        bdBool found = m_dtls.get(key, dtlsSocket);
        if (found && dtlsSocket)
        {
            status = static_cast<bdDTLSAssociationStatus>(dtlsSocket->getStatus());
            break;
        }
        if (found)
            bdLogError(LogChannel, "Invalid state! %s", addrStr);
        else
            bdLogError(LogChannel, "DTLS not found for %s", addrStr);
        status = BD_SOCKET_LOST;
        break;
    }
    case bdAddrHandle::BD_ADDR_UNRESOLVED:
        bdLogInfo(LogChannel, "Address handle unresolved. Socket lost. %s", addrStr);
        status = BD_SOCKET_LOST;
        break;
    default:
        break;
    }
    return status;
}

bdSocketRouter::bdSocketRouterStatus bdSocketRouter::getStatus()
{
    return m_status;
}

bdQoSProbe* bdSocketRouter::getQoSProber()
{
    return &m_qosProber;
}

bdAddressMap* bdSocketRouter::getAddressMap()
{
    return &m_addrMap;
}

bdSecurityKeyMap* bdSocketRouter::getKeyMap()
{
    return &m_keyStore;
}

const bdCommonAddrRef bdSocketRouter::getLocalCommonAddr() const
{
    return bdCommonAddrRef(m_localCommonAddr);
}
