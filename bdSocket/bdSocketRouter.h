// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdSocketRouter : public bdNATTravListener, public bdSecurityKeyMapListener
{
public:
    enum bdSocketRouterStatus : bdInt
    {
        BD_SOCKET_ROUTER_UNINITIALIZED = 0x0,
        BD_SOCKET_ROUTER_INITIALIZED = 0x1,
        BD_SOCKET_ROUTER_ERROR = 0x2,
    };
protected:
    bdSocket* m_socket;
    bdFastArray<bdPacketInterceptor*> m_interceptors;
    bdSocketRouter::bdSocketRouterStatus m_status;
    bdSocketRouterConfig m_config;
    bdReference<bdCommonAddr> m_localCommonAddr;
    bdAddressMap m_addrMap;
    bdSecurityKeyMap m_keyStore;
    bdQoSProbe m_qosProber;
    bdNATTravClient m_natTrav;
    bdECCKey m_ECCKey;
    bdServiceBandwidthArbitrator m_qosBandwidth;
    bdHashMap<bdEndpoint, bdDTLSAssociation*,bdEndpointHashingClass> m_dtls;
    bdHashMap<bdEndpoint, bdReference<bdAddrHandle>, bdEndpointHashingClass> m_endpointToAddrMap;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdSocketRouter();
    ~bdSocketRouter();
    bdBool init(bdSocket* socket, bdCommonAddrRef localCommonAddr, bdSocketRouterConfig& config);
    virtual void pump();
    virtual bdBool quit();
    virtual bdBool connect(bdAddrHandleRef& addrHandle);
    virtual bdBool disconnect(bdAddrHandleRef& addrHandle);
    virtual bdInt sendTo(const bdAddrHandleRef addrHandle, void* data, const bdUInt length);
    virtual bdInt receiveFrom(bdAddrHandleRef& addrHandle, void* data, const bdUInt size);
    bdInt processPacket(bdAddrHandleRef& addrHandle, bdAddr& realAddr, void* data, const bdUInt size, const bdUInt bytesTransferred, bdBool& receiving);
    void processError(bdAddr& realAddr, bdInt val, bdUInt& resetCount, bdBool& receiving);
    bdBool tryToFindConnection(void* data, const bdUInt& size, const bdSecurityID& secID, bdAddr& realAddr, bdAddrHandleRef& addrHandle, bdInt& val, bdBool checkSecID);
    bdBool tryDecryptPacket(void* data, const bdUInt& size, const bdSecurityID& secID, bdAddr& realAddr, bdAddrHandleRef& addrHandle, bdInt& val, bdBool checkSecID);
    virtual void onNATAddrDiscovery(bdCommonAddrRef remote, const bdAddr& realAddr);
    virtual void onNATAddrDiscoveryFailed(bdCommonAddrRef remote);
    bdBool setupIntroducers(const bdArray<bdAddr>& introducers);
    virtual void onSecurityKeyRemove(const bdSecurityID& id);
    void registerInterceptor(bdPacketInterceptor* const interceptor);
    void unregisterInterceptor(bdPacketInterceptor* const interceptor);
    bdInt getStatus(const bdAddrHandleRef addrHandle);
    bdSocketRouterStatus getStatus();
    bdQoSProbe* getQoSProber();
    bdAddressMap* getAddressMap();
    bdSecurityKeyMap* getKeyMap();
    const bdCommonAddrRef getLocalCommonAddr() const;
};
