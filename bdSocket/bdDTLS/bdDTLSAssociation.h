// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdDTLSAssociation
{
public:
    enum bdDTLSStatus : bdInt
    {
        BD_DTLS_CLOSED = 0x0,
        BD_DTLS_COOKIE_WAIT = 0x1,
        BD_DTLS_COOKIE_ECHOED = 0x2,
        BD_DTLS_ESTABLISHED = 0x3,
    };
protected:
    static bdBool m_cookieInited;
    static bdUByte8 m_cookieKey[16];
    bdSocket* m_socket;
    bdSecurityKeyMap* m_keyMap;
    bdECCKey* m_ECCKey;
    bdCypher3Des m_cypher;
    bdUByte8 m_sharedKey[24];
    bdAddr m_addr;
    bdReference<bdAddrHandle> m_addrHandle;
    bdDTLSAssociation::bdDTLSStatus m_state;
    bdStopwatch m_lastReceived;
    bdStopwatch m_initTimer;
    bdUByte8 m_initResends;
    bdStopwatch m_cookieTimer;
    bdUByte8 m_cookieResends;
    bdUInt16 m_localTag;
    bdUInt16 m_peerTag;
    bdSequenceNumber m_seqNum;
    bdSequenceNumberStore m_incomingSeqNums;
    bdDTLSInitAck m_initAck;
    bdSecurityID m_localId;
    bdCommonAddrRef m_localCommonAddr;
    bdAddressMap* m_addrMap;
    bdFloat32 m_receiveTimeout;
public:
    bdDTLSAssociation(bdSocket* socket, bdSecurityKeyMap* keyMap, bdECCKey* ECCKey, bdAddr* addr, bdAddrHandleRef addrHandle, bdCommonAddrRef localCommonAddr, bdAddressMap* addrMap, bdFloat32 receiveTimeout);
    ~bdDTLSAssociation();
    void connect();
    void pump();
    void reset();
    bdBool verify(bdDTLSData* dataPacket);
    bdBool calculateSharedKey(const bdUByte8* const pubKey, const bdUInt keylen, const bdSecurityID* secID);
    bdBool checkCookieValidity(const bdAddr* peerAddr, const bdDTLSCookieEcho* cookiePacket);
    bdInt sendTo(const bdAddr* addr, const void* data, const bdUInt length);
    bdInt receiveFrom(bdAddr* addr, const void* data, const bdUInt size, bdAddrHandleRef* addrHandle, void* buffer, const bdUInt bufferSize);

    bdInt handleInit(const bdAddr* addr, const void* data, const bdUInt size);
    bdInt handleInitAck(const bdAddr* addr, const void* data, const bdUInt size);
    bdInt handleCookieEcho(const bdAddr* addr, const void* data, const bdUInt size);
    bdInt handleCookieAck(const void* data, const bdUInt size);
    bdInt handleError(const bdAddr* addr, const void* data, const bdUInt size);
    bdInt handleData(bdAddr* addr, const bdUByte8* data, const bdUInt size, bdAddrHandleRef* addrHandle, bdUByte8* buffer, const bdUInt bufferSize);

    void sendInit();
    void sendInitAck(const bdAddr* addr, const bdDTLSInit* init);
    void sendCookieEcho(const bdAddr* addr);
    void sendCookieAck(const bdAddr* addr, const bdDTLSCookieEcho* cookie);
    void sendError(const bdAddr* addr, const bdSecurityID* secID, const bdDTLSError::bdDTLSErrorType* type);
    bdInt sendData(const bdAddr* addr, const void* data, const bdUInt length);

    const bdInt getStatus() const;
    const bdAddrHandleRef getAddrHandle() const;
    const bdSecurityID* getLocalSecurityId() const;
};