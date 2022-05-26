// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocket.h"

bdQoSProbe::bdQoSProbeEntryWrapper::bdQoSProbeEntryWrapper() : m_addr(), m_id(), m_key(), m_listener(NULL), m_realAddr(), m_retries(0), m_lastTry()
{
    m_lastTry.reset();
}

bdQoSProbe::bdQoSProbeEntryWrapper::bdQoSProbeEntryWrapper(const bdQoSProbe::bdQoSProbeEntryWrapper& other)
    : m_addr(other.m_addr), m_id(other.m_id), m_key(other.m_key), m_listener(other.m_listener), m_realAddr(other.m_realAddr), m_retries(other.m_retries)
{
}

bdQoSProbe::bdQoSProbeEntryWrapper::~bdQoSProbeEntryWrapper()
{
}

void* bdQoSProbe::bdQoSProbeEntryWrapper::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

void* bdQoSProbe::bdQoSProbeEntryWrapper::operator new(bdUWord __formal, void* p)
{
    return p;
}

void bdQoSProbe::bdQoSProbeEntryWrapper::operator delete(void* p)
{
}

bdQoSProbe::bdQoSProbeEntryWrapper& bdQoSProbe::bdQoSProbeEntryWrapper::operator=(const bdQoSProbe::bdQoSProbeEntryWrapper& other)
{
    m_addr = other.m_addr;
    m_id = other.m_id;
    m_key = other.m_key;
    m_listener = other.m_listener;
    m_realAddr = other.m_realAddr;
    m_retries = other.m_retries;
    m_lastTry = other.m_lastTry;
    return *this;
}

bdQoSProbe::bdQoSProbe()
    : bdNATTravListener(), bdPacketInterceptor(), m_probingTimer(), m_replyData(), m_status(BD_QOS_PROBE_UNINITIALIZED), m_probesProbbing(4u, 0.75f), m_probesResolving(4u, 0.75f)
{
}

bdQoSProbe::~bdQoSProbe()
{
    m_bandArb = NULL;
    m_natTrav = NULL;
    m_socket = NULL;
}

bdBool bdQoSProbe::init(bdSocket* socket, bdNATTravClient* natTrav, bdServiceBandwidthArbitrator* bandArb)
{
    if (m_status)
    {
        bdLogError("bdSocket/qos", "Cannot call init multiple times.");
        return false;
    }
    if (!m_socket)
    {
        bdLogError("bdSocket/qos", "A valid socket pointer is required by this class");
        return false;
    }
    if (!natTrav)
    {
        bdLogError("bdSocket/qos", "A valid NAT trav pointer is required by this class");
        return false;
    }
    if (!bandArb)
    {
        bdLogError("bdSOcket/qos", "A valid service bandwidth arbitrator pointer is required by this class");
        return false;
    }
    m_maxBandwidth = 0x4000;
    m_socket = socket;
    m_natTrav = natTrav;
    m_bandArb = bandArb;
    m_lastProbeId = 0;
    m_secid = 0;
    m_listenState = BD_QOS_OFF;
    m_probingTimer.start();
    m_bandArb->reset();
    m_bandArb->addSliceQuota((m_maxBandwidth >> 3) * 0.2000000029802322);
    m_status = BD_QOS_PROBE_INITIALIZED;
    return true;
}

bdBool bdQoSProbe::quit()
{
    if (!m_status)
    {
        return false;
    }
    cancelProbes();
    m_status = BD_QOS_PROBE_UNINITIALIZED;
    m_socket = NULL;
    m_natTrav = NULL;
    m_bandArb = NULL;
    return true;
}

void bdQoSProbe::cancelProbes()
{
    bdHashMap<bdReference<bdCommonAddr>, bdArray<bdQoSProbe::bdQoSProbeEntryWrapper>, bdHashingClass>::Iterator resit;

    if (m_status != BD_QOS_PROBE_INITIALIZED)
    {
        bdLogWarn("bdSocket/qos", "Cannot cancel probes when class is uninitialized.");
        m_probesProbbing.clear();
        m_probesResolving.clear();
        return;
    }
    bdArray<bdCommonAddrRef> probesToCancel(0u);
    for (resit = m_probesResolving.getIterator(); resit; m_probesResolving.next(resit))
    {
        probesToCancel.pushBack(m_probesResolving.getKey(resit));
    }
    for (bdUInt i = 0; i < probesToCancel.getSize(); ++i)
    {
        bdCommonAddrRef remote(probesToCancel[i]);
        m_natTrav->cancelConnect(remote);
    }
    m_probesProbbing.clear();
    m_probesResolving.clear();
}

bdBool bdQoSProbe::probe(bdQoSRemoteAddr& addr, bdQoSProbeListener* listener)
{
    return probe(bdCommonAddrRef(addr.m_addr), addr.m_id, addr.m_key, listener);
}

bdBool bdQoSProbe::probe(bdCommonAddrRef addr, const bdSecurityID& id, const bdSecurityKey& key, bdQoSProbeListener* listener)
{
    bdHashMap<bdCommonAddrRef, bdArray<bdQoSProbe::bdQoSProbeEntryWrapper>, bdHashingClass>::Iterator it;

    if (!m_status)
    {
        return false;
    }
    bdQoSProbeEntryWrapper entry;
    entry.m_addr = addr.m_ptr;
    entry.m_id = id;
    entry.m_key = key;
    entry.m_listener = listener;
    if (m_probesResolving.getIterator(addr))
    {
        bdLogInfo("bdSocket/qos", "Got multiple probes for the same address.\n");
        it = m_probesResolving.getIterator(&addr);
        m_probesResolving.getValue(it).pushBack(&entry, 1);
        m_probesResolving.releaseIterator(it);
    }
    else
    {
        bdArray<bdQoSProbe::bdQoSProbeEntryWrapper> value(1u, entry);
        m_probesResolving.put(addr, value);
    }
    return m_natTrav->connect(bdCommonAddrRef(addr), this, false);
}

void bdQoSProbe::onNATAddrDiscovery(bdCommonAddrRef remote, const bdAddr& realAddr)
{
    if (!m_probesResolving.getIterator(remote))
    {
        bdLogInfo("bdSocket/qos", "Received NAT Traversal success for unrecognized address.");
        return;
    }
    bdArray<bdQoSProbe::bdQoSProbeEntryWrapper> entries(0u);
    m_probesResolving.get(remote, entries);
    for (bdUInt i = 0; i < entries.getSize(); ++i)
    {
        bdMemcpy(&entries[i].m_realAddr, &realAddr, sizeof(entries[i].m_realAddr));
        m_probesProbbing.put(m_lastProbeId, entries[i]);
        ++m_lastProbeId;
    }
    m_probesResolving.remove(&remote);
}

void bdQoSProbe::onNATAddrDiscoveryFailed(bdCommonAddrRef remote)
{
    bdNChar8 addrStr[1024];

    if (!m_probesResolving.getIterator(remote))
    {
        bdLogInfo("bdSocket/qos", "Received NAT Traversal failure for unrecognized address.");
        return;
    }
    bdCommonAddrInfo::getBriefInfo(bdCommonAddrRef(remote.m_ptr), addrStr, sizeof(addrStr));
    bdLogInfo("bdSocket/qos", " NAT addr discovery failed to %s . QoS has therefore failed", addrStr);
    bdArray<bdQoSProbe::bdQoSProbeEntryWrapper> entries(0u);
    m_probesResolving.get(remote, entries);
    for (bdUInt i = 0; i < entries.getSize(); ++i)
    {
        entries[i].m_listener->onQoSProbeFailed(bdCommonAddrRef(entries[i].m_addr));
    }
    m_probesResolving.remove(remote);
}

bdBool bdQoSProbe::acceptPacket(bdSocket* __formal, bdAddr addr, void* data, const bdUInt size, bdUByte8 type)
{
    bdUInt deserializedSize;
    bdNChar8 srcStr[22];

    if (m_status != BD_QOS_PROBE_INITIALIZED || type < 0x28u || type > 0x31u)
    {
        return false;
    }
    addr.toString(srcStr, sizeof(srcStr));
    bdLogInfo("bdSocket/qos", "Received QoS packet from %s with type %u", srcStr, type);
    if (type == 41)
    {
        bdQoSReplyPacket packet;
        if (packet.deserialize(data, size, 0, deserializedSize) && deserializedSize == size)
        {
            return handleReply(packet, addr, size);
        }
    }
    else if (type == 40)
    {
        bdQoSRequestPacket packet;
        if (packet.deserialize(data, size, 0, deserializedSize) && deserializedSize == size)
        {
            return handleRequest(packet, addr, size);
        }
    }
    return false;
}

void bdQoSProbe::pump()
{
    bdNChar8 addrStr[1024];
    bdBool stopLoop = false;
    bdUInt32 id;
    bdHashMap<bdUInt, bdQoSProbe::bdQoSProbeEntryWrapper, bdHashingClass>::Iterator it;

    if (m_status != BD_QOS_PROBE_INITIALIZED)
    {
        bdLogWarn("bdSocket/qos", "Cannot pump class before it has bee ninitialzied.");
        return;
    }
    if (m_probingTimer.getElapsedTimeInSeconds() > 0.2f)
    {
        m_bandArb->addSliceQuota((m_maxBandwidth >> 3) * 0.2f);
        m_probingTimer.start();
    }
    if (m_probesProbbing.getSize() <= 0 || !m_bandArb->allowedSend(28))
    {
        return;
    }

    bdQueue<bdUInt> toRemove;
    for (it = m_probesProbbing.getIterator(); (it && !m_bandArb->allowedSend(28)) || stopLoop; m_probesProbbing.next(it))
    {
        id = m_probesProbbing.getKey(it);
        bdQoSProbe::bdQoSProbeEntryWrapper entry = m_probesProbbing.getValue(it);
        if (entry.m_retries && entry.m_lastTry.getElapsedTimeInSeconds() <= 0.8999999761581421)
        {
            continue;
        }

        if (entry.m_retries < 4)
        {
            sendRequest(stopLoop, entry, id);
        }
        else
        {
            bdCommonAddrInfo::getBriefInfo(bdCommonAddrRef(entry.m_addr), addrStr, sizeof(addrStr));
            bdLogWarn("bdSocket/qos", " QOS retries exceeded: QoS failed to %s ", addrStr);
            toRemove.enqueue(id);
        }
    }
    m_probesProbbing.releaseIterator(it);
    while (!toRemove.isEmpty())
    {
        bdQoSProbe::bdQoSProbeEntryWrapper entry;
        m_probesProbbing.get(toRemove.peek(), entry);
        entry.m_listener->onQoSProbeFailed(bdCommonAddrRef(&entry.m_addr));
        m_probesProbbing.remove(toRemove.peek());
        toRemove.dequeue();
    }
}

bdBool bdQoSProbe::handleRequest(bdQoSRequestPacket& packet, bdAddr& addr, bdInt32 packetSize)
{
    bdInt socketSendResult;
    bdUByte8 bufferMsg[1288];
    bdUInt dataSizeMsg;
    bdBool serializeResult;
    bdUInt dummy;

    if (m_listenState == BD_QOS_OFF)
    {
        return true;
    }
    if (!m_status)
    {
        bdLogWarn("bdSocket/qos", "Cannot handle request when in uninitialized state");
        return false;
    }
    if (m_bandArb->allowedSend(28) && packet.getSecId() == m_secid && m_bandArb->allowedSend(m_replyData.getSerializedSize() + 28))
    {
        m_replyData.setId(packet.getSecId());
        m_replyData.setTimestamp(packet.getTimestamp());
        dataSizeMsg = 0;
        serializeResult = m_replyData.serialize(bufferMsg, sizeof(bufferMsg), 0, dataSizeMsg);
        if (!serializeResult)
        {
            bdLogWarn("bdSocket/qos", "Failed to put the reply packet into a buffer.");
            return false;
        }
        if (!m_bandArb->allowedSend(dataSizeMsg + 28))
        {
            return false;
        }
        socketSendResult = m_socket->sendTo(addr, bufferMsg, dataSizeMsg);
        if (socketSendResult <= 0)
        {
            bdLogWarn("bdSocket/qos", "Failed to send reply.");
            return false;
        }
        m_bandArb->send(dataSizeMsg);
        return true;
    }
    else if (packet.getSecId() != m_secid)
    {
        bdLogWarn("bdSocket/qos", "Received a packet with invalid security ID. Dropping.");
        return false;
    }
    return false;
}

bdBool bdQoSProbe::handleReply(bdQoSReplyPacket& packet, bdAddr& addr, bdInt32 packetSize)
{
    bdHashMap<bdUInt, bdQoSProbe::bdQoSProbeEntryWrapper, bdHashingClass>::Iterator it;

    if (!m_status)
    {
        bdLogWarn("bdSocket/qos", "Cannot handle request when in uninitialized state");
        return false;
    }
    bdUInt key = packet.getId();
    if (!m_probesProbbing.containsKey(key))
    {
        bdLogWarn("bdSocket/qos", "Received probe reply with invalid id.");
        return false;
    }
    it = m_probesProbbing.getIterator(key);
    bdQoSProbe::bdQoSProbeEntryWrapper entry = m_probesProbbing.getValue(it);
    if (entry.m_realAddr != addr)
    {
        bdLogWarn("bdSocket/qos", "Received probe reply with spoofed id?");
        return false;
    }
    m_probesProbbing.remove(it);
    m_probesProbbing.releaseIterator(it);

    bdQoSProbeInfo info;
    info.m_realAddr = entry.m_realAddr;
    info.m_addr = entry.m_addr;
    info.m_data = packet.getData();
    info.m_dataSize = packet.getDataSize();
    info.m_disabled = !packet.getEnabled();
    info.m_bandwidthDown = -1;
    info.m_bandwidthUp = -1;
    entry.m_listener->onQoSProbeSuccess(info);
    return true;
}

bdBool bdQoSProbe::listen(const bdSecurityID& secid, bdUByte8* data, bdUInt dataSize)
{
    if (!setData(data, dataSize))
    {
        return false;
    }
    m_secid = shrinkSecId(secid);
    m_listenState = BD_QOS_ENABLED;
    m_replyData.setEnabled(true);
    bdLogInfo("bdSocket/qos", "Enabled QoS listener.");
    return true;
}

bdBool bdQoSProbe::setData(bdUByte8* data, bdUInt dataSize)
{
    return m_replyData.setData(data, dataSize);
}

void bdQoSProbe::disableListener()
{
    m_listenState = BD_QOS_DISABLED;
    m_replyData.setEnabled(false);
}

bdUInt bdQoSProbe::shrinkSecId(const bdSecurityID& id)
{
    bdUInt tmp = 0;
    bdUInt result = 0;

    bdBytePacker::removeBasicType<bdUInt>(id.ab, sizeof(id), 0, tmp, result);
    return result;
}

void bdQoSProbe::sendRequest(bdBool& stopProcessing, bdQoSProbe::bdQoSProbeEntryWrapper& entry, bdUInt32 id)
{
    bdBool serializeOk;
    bdUInt32 secID;
    bdUInt totalDataSize = 0;
    bdUInt packetSize = 0;
    bdNChar8 addrStr[22];
    bdUByte8 buffer[1288];

    secID = shrinkSecId(entry.m_id);
    bdQoSRequestPacket packet(secID, id);
    serializeOk = packet.serialize(buffer, sizeof(buffer), packetSize, packetSize);
    totalDataSize = packetSize + 28;
    if (!serializeOk)
    {
        bdLogWarn("bdSocket/qos", "Failed to put the probe packet into a buffer.");
        stopProcessing = true;
        return;
    }
    if (!m_bandArb->allowedSend(totalDataSize))
    {
        bdLogWarn("bdSocket/qos", " bandwidth arbitration prevents the sending of anymore data right now");
        stopProcessing = true;
        return;
    }
    entry.m_realAddr.toString(addrStr, sizeof(addrStr));
    bdLogInfo("bdSocket/qos", "Sending QOS packet to %s", addrStr);
    bdInt socketSendResult = m_socket->sendTo(entry.m_realAddr, buffer, packetSize);
    if (socketSendResult < 0)
    {
        bdLogWarn("bdSocket/qos", " Failed to send QOS packet");
    }
    ++entry.m_retries;
    entry.m_lastTry.start();
    m_bandArb->send(totalDataSize);
}
