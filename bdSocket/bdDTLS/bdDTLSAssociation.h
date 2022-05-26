// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

enum bdDTLSAssociationStatus : bdInt
{
    BD_SOCKET_IDLE = 0x0,
    BD_SOCKET_PENDING = 0x1,
    BD_SOCKET_CONNECTED = 0x2,
    BD_SOCKET_LOST = 0x3,
};

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
    bdDTLSAssociation(bdSocket* socket, bdSecurityKeyMap* keyMap, bdECCKey* ECCKey, const bdAddr& addr, bdAddrHandleRef addrHandle, bdCommonAddrRef localCommonAddr, bdAddressMap* addrMap, bdFloat32 receiveTimeout);
    ~bdDTLSAssociation();
    void connect();
    void pump();
    void reset();
    bdBool verify(bdDTLSData& dataPacket);
    bdBool calculateSharedKey(const bdUByte8* const pubKey, const bdUInt keylen, const bdSecurityID& secID);
    bdBool checkCookieValidity(const bdAddr& peerAddr, const bdDTLSCookieEcho& cookiePacket);
    bdInt sendTo(bdAddr& addr, const void* data, const bdUInt length, const bdSecurityID& secID);
    bdInt receiveFrom(bdAddr& addr, const void* data, const bdUInt size, bdAddrHandleRef& addrHandle, void* buffer, const bdUInt bufferSize);

    bdInt handleInit(bdAddr& addr, const void* data, const bdUInt size);
    bdInt handleInitAck(bdAddr& addr, const void* data, const bdUInt size);
    bdInt handleCookieEcho(bdAddr& addr, const void* data, const bdUInt size);
    bdInt handleCookieAck(const void* data, const bdUInt size);
    bdInt handleError(bdAddr& addr, const void* data, const bdUInt size);
    bdInt handleData(bdAddr& addr, const bdUByte8* data, const bdUInt size, bdAddrHandleRef& addrHandle, bdUByte8* buffer, const bdUInt bufferSize);

    void sendInit();
    void sendInitAck(bdAddr& addr, const bdDTLSInit& init);
    void sendCookieEcho(bdAddr& addr);
    void sendCookieAck(bdAddr& addr, const bdDTLSCookieEcho& cookie);
    void sendError(bdAddr& addr, const bdSecurityID& secID, const bdDTLSError::bdDTLSErrorType& type);
    bdInt sendData(bdAddr& addr, const void* data, const bdUInt length, const bdSecurityID& secID);

    const bdInt getStatus() const;
    const bdAddrHandleRef getAddrHandle() const;
    const bdSecurityID& getLocalSecurityId() const;
};
