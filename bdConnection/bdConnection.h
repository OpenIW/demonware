// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "bdSocket/bdSocket.h"

#include "bdChunk.h"
#include "bdCookie.h"
#include "bdCookieAckChunk.h"
#include "bdCookieEchoChunk.h"
#include "bdHeartbeatChunk.h"
#include "bdHeartbeatAckChunk.h"
#include "bdInitChunk.h"
#include "bdInitAckChunk.h"
#include "bdSAckChunk.h"
#include "bdShutdownChunk.h"
#include "bdShutdownCompleteChunk.h"
#include "bdShutdownAckChunk.h"

#include "bdMessage.h"
#include "bdDataChunk.h"
#include "bdPacket.h"

#include "bdReliableReceiveWindow.h"
#include "bdReliableSendWindow.h"
#include "bdUnreliableReceiveWindow.h"
#include "bdUnreliableSendWindow.h"

#include "bdConnectionStoreConfig.h"
#include "bdConnectionStatistics.h"

class bdConnection;
typedef bdReference<bdConnection> bdConnectionRef;

class bdConnectionListener
{
public:
    void* operator new(bdUWord nbytes);
    void operator delete(void* p);
    bdConnectionListener() {};
    virtual ~bdConnectionListener() {};
    virtual void onConnect(bdConnectionRef __formal) {};
    virtual void onConnectFailed(bdConnectionRef __formal) {};
    virtual void onDisconnect(bdConnectionRef __formal) {};
    virtual void onReconnect(bdConnectionRef __formal) {};
};

class bdConnection : public bdReferencable
{
public:
    enum Status : bdInt
    {
        BD_NOT_CONNECTED = 0x0,
        BD_CONNECTING = 0x1,
        BD_CONNECTED = 0x2,
        BD_DISCONNECTING = 0x3,
        BD_DISCONNECTED = 0x4,
    };
protected:
    bdReference<bdCommonAddr> m_addr;
    bdReference<bdAddrHandle> m_addrHandle;
    bdFastArray<bdConnectionListener*> m_listeners;
    bdUInt m_maxTransmissionRate;
    bdConnection::Status m_status;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdConnection();
    bdConnection(bdCommonAddrRef addr);
    virtual ~bdConnection();
    void registerListener(bdConnectionListener* const listener);
    void unregisterListener(bdConnectionListener* const listener);
    void setAddressHandle(const bdAddrHandleRef* addr);
    void setTransmissionRate(const bdUInt bytesPerSecond);
    bdUInt getTransmissionRate() const;
    const bdAddrHandleRef* getAddressHandle() const;
    bdCommonAddrRef getAddress() const;
    virtual bdBool receive(const bdUByte8*, const bdUInt) { return false; };
    virtual bdBool send(const bdMessageRef, const bdBool) { return false; };
    virtual bdBool getMessageToDispatch(bdMessageRef*) { return false; };
    virtual bdConnection::Status getStatus() const;
    virtual bdBool connect();
    virtual void disconnect();
    virtual void close();
    virtual bdFloat32 GetAvgRTT();
    virtual bdUInt getDataToSend(bdUByte8* const data, const bdUInt size) { return 0; };
};

#include "bdReceivedMessage.h"
#include "bdDispatcher.h"
#include "bdLoopbackConnection.h"
#include "bdUnicastConnection.h"
#include "bdConnectionStore.h"