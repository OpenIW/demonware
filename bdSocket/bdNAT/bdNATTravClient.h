// SPDX-License-Identifier: GPL-3.0-or-later

class bdNATTravClient : public bdPacketInterceptor
{
public:
    enum bdNATTravClientStatus : __int32
    {
        BD_NAT_TRAV_UNINITIALIZED = 0x0,
        BD_NAT_TRAV_INITIALIZED = 0x1,
        BD_NAT_TRAV_RUNNING = 0x2,
    };
protected:
    bdServiceBandwidthArbitrator* m_bandArb;
    bdArray<bdAddr> m_introducers;
    bdSocket* m_socket;
    bdHashMap<bdUInt, bdNATTravClientData, bdHashingClass> m_callbacks;
    bdStopwatch m_keepAliveTimer;
    bdUByte8 m_secretKey[28];
    bdNATTravClientStatus m_status;
    bdReference<bdCommonAddr> m_localCommonAddr;
public:
    bdNATTravClient();
    ~bdNATTravClient();
    bdBool init(bdSocket* socket, bdServiceBandwidthArbitrator* bandArb, bdCommonAddrRef localCommonAddr);
    bdBool connect(bdCommonAddrRef remote, bdNATTravListener* listener, bdBool throttle);
    void cancelConnect(bdCommonAddrRef remote);
    void receiveFrom(const bdAddr* addr, bdNATTraversalPacket* packet);
    void pump();
    void quit();
    bdBool acceptPacket(bdSocket* __formal, bdAddr addr, void* data, const bdUInt size, const bdUByte8 type);
    bdBool sendKeepAlive();
    bdBool sendStage1(bdNATTravClientData* data);
    bdBool sendStage2(bdNATTravClientData* data);
    bdBool sendStage3(bdNATTravClientData* data);
    bdBool setupIntroducers(const bdArray<bdAddr>* introducers);
    bdBool isConnectInProgress(bdCommonAddrRef remote);
    bdBool doHMac(const bdUInt32 identifier, const bdAddr* src, const bdAddr* dest, bdUByte8* data);
};