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
    bdHashMap<bdUInt,bdNATTravClientData,bdHashingClass>::Iterator it;
    unsigned int* identifier;
    bdNATTravClientData* data;
    bdNChar8 buffer[100];
    bdNChar8 commonAddrInfo[1024];

    if (!m_status)
    {
        if (!m_callbacks.getSize())
        {
            m_status = BD_NAT_TRAV_INITIALIZED;
        }
        return;
    }
    if (m_keepAliveTimer.getElapsedTimeInSeconds() > 15.0f)
    {
        sendKeepAlive();
    }
    bdQueue<bdUInt> removeQueue;
    for (it = m_callbacks.getIterator(); it; m_callbacks.next(&it))
    {
        identifier = m_callbacks.getKey(it);
        data = m_callbacks.getValue(it);
        if (data->m_throttled && data->m_age.getElapsedTimeInSeconds() > 20.0)
        {
            const_cast<bdAddr*>(data->m_remote->getPublicAddr())->toString(buffer, sizeof(buffer));
            bdLogWarn("bdSocket/nat", "Request to %s has been pending for too long (%f seconds). Allocated bandwidth inconsistent with request rate.", data->m_age.getElapsedTimeInSeconds(), buffer);
            data->callOnNATAddrDiscoveryFailed(bdCommonAddrRef(data->m_remote));
            removeQueue.enqueue(identifier);
        }
        else if (data->m_lastSent.getElapsedTimeInSeconds() > 0.89999998)
        {
            bdLogInfo("bdSocket/nat", "%f seconds since last send, retrying..", data->m_lastSent.getElapsedTimeInSeconds());
            if (data->m_state == bdNATTravClientData::BD_NTCDS_STAGE_3)
            {
                if (data->m_tries <= 3)
                {
                    bdLogInfo("bdSocket/nat", "Request timed out. Retrying. (stage 3)");
                    bdCommonAddrInfo::getBriefInfo(bdCommonAddrRef(data->m_remote), commonAddrInfo, sizeof(commonAddrInfo));
                    bdLogInfo("bdSocket/nat", commonAddrInfo);
                    sendStage3(data);
                }
                else
                {
                    bdLogInfo("bdSocket/nat", "NAT traversal failed. (Stage 3 failed)");
                    bdCommonAddrInfo::getBriefInfo(bdCommonAddrRef(data->m_remote), commonAddrInfo, sizeof(commonAddrInfo));
                    bdLogInfo("bdSocket/nat", commonAddrInfo);
                    data->callOnNATAddrDiscoveryFailed(bdCommonAddrRef(data->m_remote));
                    removeQueue.enqueue(identifier);
                }
            }
            else if (data->m_state == bdNATTravClientData::BD_NTCDS_STAGE_2)
            {
                ++data->m_tries;
                if (data->m_tries <= 4)
                {
                    bdLogInfo("bdSocket/nat", "Request timed out. Retrying. (stage 2)");
                    bdCommonAddrInfo::getBriefInfo(bdCommonAddrRef(data->m_remote), commonAddrInfo, sizeof(commonAddrInfo));
                    bdLogInfo("bdSocket/nat", commonAddrInfo);
                    sendStage2(data);
                }
                else
                {
                    bdLogInfo("bdSocket/nat", "Stage 2 failed. Starting stage 3.");
                    bdCommonAddrInfo::getBriefInfo(bdCommonAddrRef(data->m_remote), commonAddrInfo, sizeof(commonAddrInfo));
                    bdLogInfo("bdSocket/nat", commonAddrInfo);
                    data->m_tries = 0;
                    data->m_state = bdNATTravClientData::BD_NTCDS_STAGE_3;
                    sendStage3(data);
                }
            }
            else if (data->m_state == bdNATTravClientData::BD_NTCDS_STAGE_1)
            {
                if (data->m_tries <= 3)
                {
                    bdLogInfo("bdSocket/nat", "Request timed out. Retrying. (stage 1)");
                    bdCommonAddrInfo::getBriefInfo(bdCommonAddrRef(data->m_remote), commonAddrInfo, sizeof(commonAddrInfo));
                    bdLogInfo("bdSocket/nat", commonAddrInfo);
                    sendStage1(data);
                }
                else
                {
                    if (!const_cast<bdAddr*>(data->m_remote->getPublicAddr())->getAddress()->isValid() || m_introducers.isEmpty())
                    {
                        bdLogWarn("bdSocket/nat", "Address lookup failed. (Stage 1 failed. No public address so finished.)");
                        bdCommonAddrInfo::getBriefInfo(bdCommonAddrRef(data->m_remote), commonAddrInfo, sizeof(commonAddrInfo));
                        bdLogInfo("bdSocket/nat", commonAddrInfo);
                        data->callOnNATAddrDiscoveryFailed(bdCommonAddrRef(data->m_remote));
                        removeQueue.enqueue(identifier);
                    }
                    else
                    {
                        bdLogInfo("bdSocket/nat", "Stage 1 failed. Starting stage 2.");
                        bdCommonAddrInfo::getBriefInfo(bdCommonAddrRef(data->m_remote), commonAddrInfo, sizeof(commonAddrInfo));
                        bdLogInfo("bdSocket/nat", commonAddrInfo);
                        data->m_tries = 0;
                        data->m_state = bdNATTravClientData::BD_NTCDS_STAGE_2;
                        sendStage2(data);
                    }
                }
            }
            else
            {
                bdLogWarn("bdSocket/nat", "Invalid state.");
            }
        }
        m_callbacks.releaseIterator(NULL);
        while (!removeQueue.isEmpty())
        {
            m_callbacks.remove(removeQueue.peek());
            removeQueue.dequeue();
        }
    }
    if (!m_callbacks.getSize())
    {
        m_status = BD_NAT_TRAV_INITIALIZED;
    }
}

bdBool bdNATTravClient::quit()
{
    bdHashMap<bdUInt, bdNATTravClientData, bdHashingClass>::Iterator it;
    bdNChar8 tmp[1024];

    if (m_status == BD_NAT_TRAV_INITIALIZED)
    {
        m_socket = NULL;
        m_bandArb = NULL;
        m_status = BD_NAT_TRAV_UNINITIALIZED;
        m_introducers.clear();
        return true;
    }
    else if (m_status == BD_NAT_TRAV_RUNNING)
    {
        it = m_callbacks.getIterator();
        while (it)
        {
            bdNATTravClientData data(m_callbacks.getValue(it));
            bdCommonAddrInfo::getBriefInfo(bdCommonAddrRef(data.m_remote), tmp, sizeof(tmp));
            bdLogInfo("bdSocket/nat", "NAT traversal to %s failed.", tmp);
            m_callbacks.next(&it);
        }
        m_callbacks.releaseIterator(NULL);
        m_introducers.clear();
        m_socket = NULL;
        m_bandArb = NULL;
        m_status = BD_NAT_TRAV_INITIALIZED;
        return true;
    }
    return false;
}

bdBool bdNATTravClient::acceptPacket(bdSocket* __formal, bdAddr addr, void* data, const bdUInt size, const bdUByte8 type)
{
    bdUInt newOffset;

    if (!m_status)
    {
        return false;
    }
    if (type >= 10 && type <= 19)
    {
        bdNATTraversalPacket packet;
        if (packet.deserialize(data, size, 0, &newOffset) &&
            packet.getType() >= 10 &&
            packet.getType() <= 19)
        {
            receiveFrom(&addr, &packet);
            return true;
        }
    }
    return false;
}

bdBool bdNATTravClient::sendKeepAlive()
{
    bdUInt newOffset;
    bdUByte8 type;
    bdUByte8 buffer[1288];
    bdInt sendResult;

    m_keepAliveTimer.start();
    if (m_introducers.isEmpty())
    {
        return true;
    }
    type = 14;
    bdNATTraversalPacket packet(&type, 0, &bdAddr(), &bdAddr());
    if (packet.serialize(buffer, sizeof(buffer), 0, &newOffset))
    {
        for (bdInt i = 0; i < m_introducers.getSize(); ++i)
        {
            sendResult = m_socket->sendTo(m_introducers[i], buffer, newOffset);
            if (sendResult != newOffset)
            {
                bdLogWarn("bdSocket/nat", "Failed to send. Invalid Socket?");
                return false;
            }
        }
        return true;
    }
    return false;
}

bdBool bdNATTravClient::sendStage1(bdNATTravClientData* data)
{
    bdUInt identifier;
    bdUByte8 type;
    bdUByte8 hmacTemp[10];
    bdUByte8 buffer[1288];
    bdNChar8 dstStr[22];

    if (data->m_throttled && !m_bandArb->allowedSend(28u))
    {
        return false;
    }
    data->m_lastSent.start();
    ++data->m_tries;
    bdCommonAddrRef local(data->m_local);
    bdCommonAddrRef remote(data->m_remote);
    identifier = remote->getHash();
    bdAddr src(local->getLocalAddrByIndex(0));
    bdArray<bdAddr> destinations(remote->getLocalAddrs());
    if (const_cast<bdAddr*>(remote->getPublicAddr())->getAddress()->isValid())
    {
        destinations.pushBack(remote->getPublicAddr());
    }
    bdBool sent = true;
    for (bdUInt i = 0; i < destinations.getSize(); ++i)
    {
        bdAddr dst(destinations[i]);
        type = 13;
        bdNATTraversalPacket packet(&type, identifier, &src, &dst);
        if (!doHMac(identifier, packet.getAddrSrc(), packet.getAddrDest(), hmacTemp))
        {
            bdLogWarn("bdSocket/nat", "Failed to do hmac.");
            continue;
        }
        packet.setHMAC(hmacTemp, sizeof(hmacTemp));
        bdUInt serializedSize = 0;
        packet.serialize(buffer, sizeof(buffer), 0, &serializedSize);
        if (data->m_throttled)
        {
            m_bandArb->send(packet.getSize() + 28);
        }
        bdInt sendResult = m_socket->sendTo(&dst, buffer, serializedSize);
        if (sendResult != serializedSize)
        {
            bdLogWarn("bdSocket/nat", "Failed to send. Invalid socket?");
            sent = false;
            continue;
        }
        dst.toString(dstStr, sizeof(dstStr));
        bdLogInfo("bdSocket/nat", "sent INTRO REQ to %s", dstStr);
    }
    return sent;
}

bdBool bdNATTravClient::sendStage2(bdNATTravClientData* data)
{
    bdUInt identifier;
    bdUByte8 type;
    bdUByte8 hmacTemp[10];
    bdUByte8 buffer[1288];
    bdNChar8 dstStr[22];
    bdNChar8 srcStr[22];
    bdNChar8 intr[22];

    if (data->m_throttled && !m_bandArb->allowedSend(28u))
    {
        return false;
    }
    data->m_lastSent.start();
    ++data->m_tries;
    bdCommonAddrRef local(data->m_local);
    bdCommonAddrRef remote(data->m_remote);
    identifier = remote->getHash();
    bdAddr src(local->getPublicAddr());
    bdAddr dst(remote->getPublicAddr());
    type = 10;
    bdNATTraversalPacket packet(&type, identifier, &src, &dst);
    if (!doHMac(identifier, packet.getAddrSrc(), packet.getAddrDest(), hmacTemp))
    {
        bdLogWarn("bdSocket/nat", "Failed to do hmac.");
        return false;
    }
    packet.setHMAC(hmacTemp, sizeof(hmacTemp));
    bdUInt serialziedSize = 0;
    packet.serialize(buffer, sizeof(buffer), 0, &serialziedSize);
    if (data->m_throttled)
    {
        m_bandArb->send(packet.getSize() + 28);
    }
    bdInt sendResult = m_socket->sendTo(m_introducers[0], buffer, serialziedSize);
    if (sendResult != serialziedSize)
    {
        bdLogWarn("bdSocket/nat", "Failed to send. Invalid socket?");
        return false;
    }
    m_introducers[0]->toString(intr, sizeof(intr));
    const_cast<bdAddr*>(packet.getAddrDest())->toString(dstStr, sizeof(dstStr));
    const_cast<bdAddr*>(packet.getAddrSrc())->toString(srcStr, sizeof(srcStr));
    bdLogInfo("bdSocket/nat", "sent INTRO NAT req to %s for %s from %s", intr, dstStr, srcStr);
    return true;
}

bdBool bdNATTravClient::sendStage3(bdNATTravClientData* data)
{
    return sendStage1(data);
}

bdBool bdNATTravClient::setupIntroducers(const bdArray<bdAddr>* introducers)
{
    if (!m_status)
    {
        return false;
    }
    if (m_introducers.getSize())
    {
        bdLogWarn("bdSocket/nat", "Overwriting existing introducer entries.");
    }
    m_introducers = introducers;
    return true;
}

bdBool bdNATTravClient::isConnectInProgress(bdCommonAddrRef remote)
{
    bdUInt32 identifier = remote.m_ptr->getHash();
    return m_callbacks.containsKey(&identifier);
}

bdBool bdNATTravClient::doHMac(const bdUInt32 identifier, const bdAddr* src, const bdAddr* dest, bdUByte8* data)
{
    bdUInt hmacLength;
    bdUInt offset;
    bdUByte8 ident[4];
    bdUByte8 addrBuffer[136];

    offset = 0;
    if (!bdBytePacker::appendBasicType<bdUInt32>(reinterpret_cast<void*>(ident), sizeof(ident), 0, &offset, const_cast<bdUInt32*>(&identifier)))
    {
        return false;
    }
    bdHMacSHA1 hmac(m_secretKey, sizeof(m_secretKey));
    if (!hmac.process(ident, sizeof(ident)))
    {
        return false;
    }
    offset = 0;
    if (!const_cast<bdAddr*>(src)->serialize(addrBuffer, sizeof(addrBuffer), 0, &offset))
    {
        return false;
    }
    if (!hmac.process(addrBuffer, offset))
    {
        return false;
    }
    offset = 0;
    if (!const_cast<bdAddr*>(dest)->serialize(addrBuffer, sizeof(addrBuffer), 0, &offset));
    {
        return false;
    }
    if (!hmac.process(addrBuffer, offset))
    {
        return false;
    }
    hmacLength = 10;
    return hmac.getData(data, &hmacLength);
}
