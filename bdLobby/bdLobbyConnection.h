// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#define BD_MSG_SIZE_BUFFER_SIZE 4
#define plaintextHeaderSize 5

class bdPendingBufferTransfer : public bdReferencable
{
protected:
    bdByteBufferRef m_buffer;
    bdUByte8* m_txPtr;
    bdUInt32 m_txAvail;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdPendingBufferTransfer(bdTaskByteBufferRef buffer, bdUInt totalSize);
    bdPendingBufferTransfer(const bdPendingBufferTransfer* other);
    bdPendingBufferTransfer(bdByteBufferRef buffer, bdUInt totalSize);
    ~bdPendingBufferTransfer();
    bdUInt32 getAvail() const;
    bdUByte8* getData() const;
    bdUInt32 updateTransfer(bdUInt32 amountTransfered);
};

typedef bdReference<bdPendingBufferTransfer> bdPendingBufferTransferRef;

class bdLobbyConnection : public bdReferencable
{
public:
    enum Status
    {
        BD_NOT_CONNECTED = 0,
        BD_CONNECTING = 1,
        BD_CONNECTED = 2,
        BD_DISCONNECTING = 3,
        BD_DISCONNECTED = 4
    };
    enum RecvState : bdInt
    {
        BD_READ_INIT = 0x0,
        BD_READ_SIZE = 0x1,
        BD_READ_ENCRYPT = 0x2,
        BD_READ_MESSAGE = 0x3,
        BD_READ_COMPLETE = 0x4,
    };
protected:
    bdCommonAddrRef m_addr;
    bdUInt m_maxSendMessageSize;
    bdUInt m_maxRecvMessageSize;
    bdLobbyConnection::RecvState m_recvState;
    bdUByte8 m_msgSizeBuffer[BD_MSG_SIZE_BUFFER_SIZE];
    bdUInt m_recvCount;
    bdUByte8 m_recvEncryptType;
    bdUInt m_messageSize;
    bdTaskByteBufferRef m_recvMessage;
    bdPendingBufferTransferRef m_recvTransfer;
    bdQueue<bdPendingBufferTransfer> m_outgoingBuffers;
    bdStreamSocket m_socket;
    bdLobbyConnection::Status m_status;
    bdLobbyConnectionListener* m_connectionListener;
    bdCypher3Des m_cypher;
    bdUByte8 m_sessionKey[24];
    bdUInt m_messageCount;
    bdStopwatch m_keepAliveTimer;
    bdStopwatch m_lastReceivedTimer;
    bdStopwatch m_asyncConnectTimer;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdLobbyConnection(bdCommonAddrRef lobbyAddr, bdUWord sendBufSize, bdUWord recvBufSize, bdLobbyConnectionListener* connectionListener);
    bdLobbyConnection(bdCommonAddrRef lobbyAddr, bdLobbyConnectionListener* connectionListener);
    ~bdLobbyConnection();
    virtual bdBool sendTask(bdTaskByteBufferRef message, bdUInt messageSize, bdBool encrypt);
    virtual bdBool send(bdUByte8* message, bdUInt messageSize, bdBool encrypt);
    void sendRaw(bdUByte8* buffer, const bdUInt32 bufferSize);
    void sendRaw(bdByteBufferRef buffer, const bdUInt32 bufferSize);
    bdBool getMessageToDispatch(bdUByte8* type, bdByteBufferRef* payload);
    bdBool getMessageToDispatch(bdUByte8* type, bdBitBufferRef* payload);
    void setSessionKey(const bdUByte8* const sesssionKey);
    bdBool connect(bdAuthInfo* authInfo);
    void disconnect();
    void close();
    bdBool pump();
    bdLobbyConnection::Status getStatus() const;
    bdUInt getReceiveBufferSize() const;
protected:
    void callListenersConnect(const bdBool success);
    void callListenersDisconnect();
    bdBool recvMessageData();
    bdInt recvMessageSize();
    bdInt recvEncryptType();
    bdBool receivedFullMessage();
};
