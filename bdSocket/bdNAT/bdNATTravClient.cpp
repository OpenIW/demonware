// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocketRouter.h"

bdNATTravClient::bdNATTravClient() : bdPacketInterceptor(), m_introducers(0u), m_callbacks(4u, 0.75), m_keepAliveTimer(), m_status(BD_NAT_TRAV_UNINITIALIZED), m_localCommonAddr()
{
}

bdNATTravClient::~bdNATTravClient()
{
    bdHashMap<bdUInt, bdNATTravClientData, bdHashingClass>::Iterator it;
    bdNChar8 tmp[1024];

    it = m_callbacks.getIterator();
    while (it)
    {
        bdNATTravClientData data(m_callbacks.getValue(it));
        bdCommonAddrInfo::getBriefInfo(&bdCommonAddrRef(data.m_remote), tmp, sizeof(tmp));
        bdLogInfo("bdSocket/nat", "NAT traversal to %s failed.", tmp);
        data.callOnNATAddrDiscoveryFailed(bdCommonAddrRef(data.m_remote));
        m_callbacks.next(&it);
    }
    m_callbacks.releaseIterator(NULL);
    m_callbacks.clear();
}

bdBool bdNATTravClient::init(bdSocket* socket, bdServiceBandwidthArbitrator* bandArb, bdCommonAddrRef localCommonAddr)
{
    if (m_status)
    {
        bdLogError("bdSocket/nat", "Cannot call init multiple times.");
        return false;
    }
    if (!socket)
    {
        bdLogError("bdSocket/nat", "A valid socket pointer is required by this class");
        return false;
    }
    if (!bandArb)
    {
        bdLogError("bdSocket/nat", "A valid service bandwidth arbitrator pointer is required by this class");
        return false;
    }
    if (localCommonAddr.isNull())
    {
        bdLogError("bdSocket/nat", "A valid local common addr ref is required by this class");
        return false;
    }
    m_socket = socket;
    m_bandArb = bandArb;
    m_localCommonAddr = localCommonAddr.m_ptr;
    bdSingleton<bdTrulyRandomImpl>::getInstance()->getRandomUByte8(m_secretKey, sizeof(m_secretKey));
    m_status = BD_NAT_TRAV_INITIALIZED;
    return sendKeepAlive();
}

bdBool bdNATTravClient::connect(bdCommonAddrRef remote, bdNATTravListener* listener, bdBool throttle)
{
    bdHashMap<bdUInt, bdNATTravClientData, bdHashingClass>::Iterator it;
    bdNChar8 commonAddrInfo[1024];
    bdNChar8 destAddr[1024];

    if (!m_status)
    {
        bdLogError("bdSocket/nat", "Cannot call connect until this class has been initialized.");
        return false;
    }
    bdUInt32 identifier = remote.m_ptr->getHash();
    if (m_callbacks.containsKey(&identifier))
    {
        it = m_callbacks.getIterator(&identifier);
        bdNATTravClientData* data = m_callbacks.getValue(it);
        if (data->m_secondaryListener)
        {
            bdCommonAddrInfo::getBriefInfo(bdCommonAddrRef(remote.m_ptr), commonAddrInfo, sizeof(commonAddrInfo));
            bdLogWarn("bdSocket/nat", "Third connect request to %s. Ignoring.", commonAddrInfo);
            return false;
        }
        data->m_secondaryListener = listener;
        m_callbacks.releaseIterator(it);
    }
    else
    {
        bdNATTravClientData data(&bdCommonAddrRef(m_localCommonAddr), &bdCommonAddrRef(remote.m_ptr), listener);
        data.m_throttled = throttle;
        data.m_age.start();
        bdCommonAddrInfo::getInfo(bdCommonAddrRef(remote.m_ptr), destAddr, sizeof(destAddr));
        bdLogInfo("bdSocket/nat", "Starting NAT trav to %s", destAddr);
        if (!sendStage1(&data))
        {
            bdLogWarn("bdSocket/nat", "Initial stage 1 send failed. Continuing anyway");
        }
        if (m_callbacks.put(&identifier, &data))
        {
            m_status = BD_NAT_TRAV_RUNNING;
        }
        else
        {
            bdLogError("bdSocket/nat", "Failed to put address in map");
            bdCommonAddrInfo::getBriefInfo(bdCommonAddrRef(remote.m_ptr), commonAddrInfo, sizeof(commonAddrInfo));
            bdLogInfo("bdSocket/nat", commonAddrInfo);
            return false;
        }
    }
    return true;
}

void bdNATTravClient::cancelConnect(bdCommonAddrRef remote)
{
    bdUInt identifier;
    bdNChar8 commonAddrInfo[1024];

    if (remote.isNull())
    {
        bdLogWarn("bdSocket/nat", "Connect canceled to null common addr ref.");
        return;
    }
    identifier = remote->getHash();
    bdNATTravClientData callback;
    if (m_callbacks.remove(&identifier, &callback))
    {
        callback.callOnNATAddrDiscoveryFailed(bdCommonAddrRef(remote.m_ptr));
    }
    else
    {
        bdCommonAddrInfo::getBriefInfo(bdCommonAddrRef(remote.m_ptr), commonAddrInfo, sizeof(commonAddrInfo));
        bdLogWarn("bdSocket/nat", "Connect canceled to unknown remote host %s. Ignoring.", commonAddrInfo);
    }
}

void bdNATTravClient::receiveFrom(const bdAddr* addr, bdNATTraversalPacket* packet)
{
    bdUByte8 hmacBuffer[10];
    bdUByte8 buffer[1288];
    bdNChar8 dst[22];
    bdNChar8 src[22];
    bdNChar8 lookup[22];
    bdUInt length;
    bdInt sendResult;

    switch (packet->getType())
    {
    case 10:
    {
        bdLogWarn("bdSocket/nat", "Received server packet in client code.");
        break;
    }
    case 11:
    {
        bdAddr dest(packet->getAddrSrc());
        packet->setType(12u);
        if (!packet->serialize(buffer, sizeof(buffer), 0, &length))
        {
            bdLogError("bdSocket/nat", "Cannot serialize the packet.");
            break;
        }
        sendResult = m_socket->sendTo(&dest, buffer, length);
        dest.toString(dst, sizeof(dst));
        if (sendResult <= 0)
        {
            bdLogError("bdSocket/nat", "Failed to send INTRO REPLY to %s. Socket error %i", dst);
            break;
        }
        bdLogInfo("bdSocket/nat", "sent INTRO REPLY to %s", dst);
        break;
    }
    case 12:
    {
        bdLogInfo("bdSocket/nat", "Received NAT Trav reply.");
        bdUInt32 identifier = packet->getIdentifier();
        if (!doHMac(packet->getIdentifier(), packet->getAddrSrc(), packet->getAddrDest(), hmacBuffer) || (bdMemcmp(hmacBuffer, packet->getHMAC(), sizeof(hmacBuffer))))
        {
            bdLogWarn("bdSocket/nat", "Packed was tampered with, discarding.");
            break;
        }
        const_cast<bdAddr*>(addr)->toString(src, sizeof(src));
        const_cast<bdAddr*>(packet->getAddrDest())->toString(lookup, sizeof(lookup));
        bdLogInfo("bdSocket/nat", "discovered addr for %s: %s", lookup, src);
        bdNATTravClientData data;
        if (!m_callbacks.remove(&identifier, &data))
        {
            bdLogInfo("bdSocket/nat", "Failed to find identifier in callback table. This is ok if we have already received a reply for this search.");
            break;
        }
        data.callOnNATAddrDiscovery(bdCommonAddrRef(data.m_remote), addr);
        break;
    }
    case 13:
    {
        if (packet->getIdentifier() != m_localCommonAddr->getHash())
        {
            const_cast<bdAddr*>(addr)->toString(src, sizeof(src));
            bdLogInfo("bdSocket/nat", "ignored request from %s", src);
            break;
        }
        bdAddr dest(addr);
        packet->setType(12u);
        if (!packet->serialize(buffer, sizeof(buffer), 0, &length))
        {
            bdLogError("bdSocket/nat", "Cannot serialize the packet.");
            break;
        }
        sendResult = m_socket->sendTo(&dest, buffer, length);
        dest.toString(dst, sizeof(dst));
        if (sendResult <= 0)
        {
            bdLogError("bdSocket/nat", "Failed to send INTRO REPLY to %s. Socket error %i", dst, sendResult);
            break;
        }
        bdLogInfo("bdSocket/nat", "sent INTRO REPLY to %s", dst);
        break;
    }
    default:
    {
        break;
    }
    }
}

void bdNATTravClient::pump()
{
}

void bdNATTravClient::quit()
{
}

bdBool bdNATTravClient::acceptPacket(bdSocket* __formal, bdAddr addr, void* data, const bdUInt size, const bdUByte8 type)
{
    return bdBool();
}

bdBool bdNATTravClient::sendKeepAlive()
{
    return bdBool();
}

bdBool bdNATTravClient::sendStage1(bdNATTravClientData* data)
{
    return bdBool();
}

bdBool bdNATTravClient::sendStage2(bdNATTravClientData* data)
{
    return bdBool();
}

bdBool bdNATTravClient::setupIntroducers(const bdArray<bdAddr>* introducers)
{
    return bdBool();
}

bdBool bdNATTravClient::isConnectInProgress(bdCommonAddrRef localCommonAddr)
{
    return bdBool();
}

bdBool bdNATTravClient::doHMac(const bdUInt32 identifier, const bdAddr* src, const bdAddr* dest, bdUByte8* data)
{
    return bdBool();
}
