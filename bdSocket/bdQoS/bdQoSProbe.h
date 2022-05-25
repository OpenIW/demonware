// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdQoSProbe : public bdNATTravListener, public bdPacketInterceptor
{
public:
    enum bdQoSListenerState : bdInt
    {
        BD_QOS_OFF = 0x0,
        BD_QOS_DISABLED = 0x1,
        BD_QOS_ENABLED = 0x2,
    };
    enum bdQoSProbeStatus : bdInt
    {
        BD_QOS_PROBE_UNINITIALIZED = 0x0,
        BD_QOS_PROBE_INITIALIZED = 0x1,
        BD_QOS_PROBE_RUNNING = 0x2,
    };
    class bdQoSProbeEntryWrapper
    {
    public:
        bdCommonAddrRef m_addr;
        bdSecurityID m_id;
        bdSecurityKey m_key;
        bdQoSProbeListener* m_listener;
        bdAddr m_realAddr;
        unsigned int m_retries;
        bdStopwatch m_lastTry;

        void* operator new(bdUWord nbytes);
        void* operator new(bdUWord __formal, void* p);
        void operator delete(void* p);
        bdQoSProbeEntryWrapper();
        bdQoSProbeEntryWrapper(const bdQoSProbe::bdQoSProbeEntryWrapper& other);
        ~bdQoSProbeEntryWrapper();
        bdQoSProbe::bdQoSProbeEntryWrapper& bdQoSProbe::bdQoSProbeEntryWrapper::operator=(bdQoSProbe::bdQoSProbeEntryWrapper& other);
        void* operator new(bdUWord nbytes);
        void* operator new(bdUWord __formal, void* p);
    };
protected:
    unsigned int m_maxBandwidth;
    bdSocket* m_socket;
    bdNATTravClient* m_natTrav;
    bdServiceBandwidthArbitrator* m_bandArb;
    bdUInt m_lastProbeId;
    bdStopwatch m_probingTimer;
    bdUInt m_secid;
    bdQoSReplyPacket m_replyData;
    bdQoSListenerState m_listenState;
    bdQoSProbe::bdQoSProbeStatus m_status;
    bdHashMap<bdCommonAddrRef, bdArray<bdQoSProbe::bdQoSProbeEntryWrapper>, bdHashingClass> m_probesResolving;
    bdHashMap<bdUInt, bdQoSProbe::bdQoSProbeEntryWrapper, bdHashingClass> m_probesProbbing;
public:
    bdQoSProbe();
    ~bdQoSProbe();
    bdBool init(bdSocket* socket, bdNATTravClient* natTrav, bdServiceBandwidthArbitrator* bandArb);
    bdBool quit();
    void cancelProbes();
    bdBool probe(bdQoSRemoteAddr& addr, bdQoSProbeListener* listener);
    bdBool probe(bdCommonAddrRef addr, const bdSecurityID& id, const bdSecurityKey& key, bdQoSProbeListener* listener);
    void onNATAddrDiscovery(bdCommonAddrRef remote, const bdAddr& realAddr);
    void onNATAddrDiscoveryFailed(bdCommonAddrRef remote);
    bdBool acceptPacket(bdSocket* __formal, bdAddr addr, void* data, const bdUInt size, bdUByte8 type);
    void pump();
    bdBool handleRequest(bdQoSRequestPacket& packet, bdAddr& addr, bdInt32 packetSize);
    bdBool handleReply(bdQoSReplyPacket& packet, bdAddr& addr, bdInt32 packetSize);
    bdBool listen(const bdSecurityID& secid, bdUByte8* data, bdUInt dataSize);
    bdBool setData(bdUByte8* data, bdUInt dataSize);
    void disableListener();
    bdUInt shrinkSecId(const bdSecurityID& id);
    void sendRequest(bdBool& stopProcessing, bdQoSProbe::bdQoSProbeEntryWrapper& entry, bdUInt32 id);
};
