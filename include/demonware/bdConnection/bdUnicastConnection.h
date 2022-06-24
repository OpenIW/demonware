// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdUnicastConnection : public bdConnection
{
public:
    enum bdUnicastConnectionStatus : bdInt
    {
        BD_UC_CLOSED = 0x0,
        BD_UC_COOKIE_WAIT = 0x1,
        BD_UC_COOKIE_ECHOED = 0x2,
        BD_UC_ESTABLISHED = 0x3,
        BD_UC_SHUTDOWN_PENDING = 0x4,
        BD_UC_SHUTDOWN_SENT = 0x5,
        BD_UC_SHUTDOWN_RECEIVED = 0x6,
        BD_UC_SHUTDOWN_ACK_SENT = 0x7,
    };
    class bdControlChunkStore
    {
    public:
        bdChunkRef m_chunk;
        bdBool m_lone;

        bdControlChunkStore(bdChunkRef chunk, bdBool lone);
        bdControlChunkStore(const bdUnicastConnection::bdControlChunkStore& other);
    };
protected:
    bdConnectionStatistics m_stats;
    bdReliableSendWindow* m_reliableSendWindow;
    bdReliableReceiveWindow* m_reliableRecvWindow;
    bdUnreliableSendWindow m_unreliableSendWindow;
    bdUnreliableReceiveWindow m_unreliableReceiveWindow;
    bdQueue<bdControlChunkStore> m_outQueue;
    bdStopwatch m_sendTimer;
    bdStopwatch m_receiveTimer;
    bdUnicastConnection::bdUnicastConnectionStatus m_state;
    bdUInt m_localTag;
    bdUInt m_peerTag;
    bdStopwatch m_initTimer;
    bdUByte8 m_initResends;
    bdStopwatch m_cookieTimer;
    bdUByte8 m_cookieResends;
    bdStopwatch m_shutdownTimer;
    bdUByte8 m_shutdownResends;
    bdStopwatch m_shutdownGuard;
    bdReference<bdInitAckChunk> m_initAckChunk;
    bdFloat32 m_smoothedRTT;
    bdFloat32 m_RTTVariation;
    bdAddressMap* m_addrMap;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdUnicastConnection(bdAddressMap* addrMap);
    bdUnicastConnection(bdCommonAddrRef dest, bdAddressMap* addrMap);
    ~bdUnicastConnection();
    bdBool connect();
    void disconnect();
    bdBool send(bdReference<bdMessage> message, bdBool reliable);
    bdConnection::Status getStatus();
    void close();
    bdConnectionStatistics* getStats();
    bdFloat32 GetAvgRTT();
    bdBool receive(bdUByte8* buffer, const bdUInt bufferSize);
    bdUInt getDataToSend(bdUByte8* const buffer, const bdUInt bufferSize);
    bdBool getMessageToDispatch(bdMessageRef& message);
    bdBool handleData(bdChunkRef& chunk);
    bdBool handleSAck(bdChunkRef& chunk);
    bdBool handleInit(bdChunkRef& chunk);
    bdBool handleInitAck(bdChunkRef& chunk, const bdUInt vtag);
    bdBool handleCookieEcho(bdChunkRef& chunk, const bdUInt vtag);
    bdBool handleCookieAck(bdChunkRef& chunk, const bdUInt vtag);
    bdBool handleHeartbeat(bdChunkRef& chunk);
    bdBool handleHeartbeatAck(bdChunkRef& chunk);
    bdBool handleShutdown(bdChunkRef& chunk);
    bdBool handleShutdownAck(bdChunkRef& chunk);
    bdBool handleShutdownComplete(bdChunkRef& chunk);
    bdBool sendInit();
    bdBool sendInitAck(bdReference<bdInitChunk> chunk);
    bdBool sendCookieEcho(bdReference<bdInitAckChunk> chunk);
    bdBool sendCookieAck();
    bdBool sendHeartbeat(bdReference<bdInitChunk> chunk);
    bdBool sendHeartbeatAck(bdReference<bdInitChunk> chunk);
    bdBool sendShutdown();
    bdBool sendShutdownAck();
    bdBool sendShutdownComplete();
    void reset();
    void callListenersConnect(const bdBool success);
    void callListenersDisconnect();
    void callListenersReconnect();
    bdBool windowsEmpty();
};
