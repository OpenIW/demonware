// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdLobby/bdLobby.h"

// bdPendingBufferTransfer

void bdPendingBufferTransfer::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdPendingBufferTransfer::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdPendingBufferTransfer::bdPendingBufferTransfer(bdTaskByteBufferRef buffer, bdUInt totalSize)
    : bdReferencable(), m_buffer(*buffer), m_txPtr((*buffer)->getHeaderStart()), m_txAvail(m_txPtr ? totalSize : 0)
{
}

bdPendingBufferTransfer::bdPendingBufferTransfer(const bdPendingBufferTransfer* other)
    : bdReferencable(), m_buffer(&other->m_buffer), m_txPtr(other->m_txPtr), m_txAvail(other->m_txAvail)
{
}

bdPendingBufferTransfer::bdPendingBufferTransfer(bdByteBufferRef buffer, bdUInt totalSize)
    : bdReferencable(), m_buffer(*buffer), m_txPtr(buffer->getData()), m_txAvail(m_txPtr ? totalSize : 0)
{
}

bdPendingBufferTransfer::~bdPendingBufferTransfer()
{
}

bdUInt32 bdPendingBufferTransfer::getAvail() const
{
    return m_txAvail;
}

bdUByte8* bdPendingBufferTransfer::getData() const
{
    return m_txPtr;
}

bdUInt32 bdPendingBufferTransfer::updateTransfer(bdUInt32 amountTransfered)
{
    bdAssert(amountTransfered <= m_txAvail, "Transfered too much on lobby task");
    m_txPtr += m_txAvail <= amountTransfered ? m_txAvail : amountTransfered;
    m_txAvail -= m_txAvail <= amountTransfered ? m_txAvail : amountTransfered;
    return m_txAvail;
}

// bdLobbyConnection

void bdLobbyConnection::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdLobbyConnection::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdLobbyConnection::bdLobbyConnection(bdCommonAddrRef lobbyAddr, bdUWord sendBufSize, bdUWord recvBufSize, bdLobbyConnectionListener* connectionListener)
    : bdReferencable(), m_addr(lobbyAddr), m_maxSendMessageSize(sendBufSize), m_maxRecvMessageSize(recvBufSize), m_recvState(BD_READ_INIT), m_recvCount(0), m_recvEncryptType(0),
    m_messageSize(0), m_recvMessage(), m_recvTransfer(), m_outgoingBuffers(), m_socket(), m_status(BD_NOT_CONNECTED), m_connectionListener(connectionListener), m_cypher(),
    m_messageCount(0), m_keepAliveTimer(), m_lastReceivedTimer(), m_asyncConnectTimer()
{
    m_socket.create(false);
}

bdLobbyConnection::bdLobbyConnection(bdCommonAddrRef lobbyAddr, bdLobbyConnectionListener* connectionListener)
    : bdReferencable(), m_addr(lobbyAddr), m_maxSendMessageSize(0xFFFF), m_maxRecvMessageSize(0xFFFF), m_recvState(BD_READ_INIT), m_recvCount(0), m_recvEncryptType(0),
    m_messageSize(0), m_recvMessage(), m_recvTransfer(), m_outgoingBuffers(), m_socket(), m_status(BD_NOT_CONNECTED), m_connectionListener(connectionListener), m_cypher(),
    m_messageCount(0), m_keepAliveTimer(), m_lastReceivedTimer(), m_asyncConnectTimer()
{
    m_socket.create(false);
}

bdLobbyConnection::~bdLobbyConnection()
{
    m_socket.close();
}

bdBool bdLobbyConnection::sendTask(bdTaskByteBufferRef message, bdUInt messageSize, bdBool encrypt)
{
    bdUByte8 initialVector[24];
    bdUByte8 signature[4];
    bdUByte8* writePtr;

    if (messageSize > m_maxSendMessageSize)
    {
        bdLogError("bdLobby/bdLobbyConnection", "Message too big for outgoing buffer.");
        pump();
        return false;
    }
    if (m_status != BD_CONNECTED && m_status != BD_CONNECTING)
    {
        pump();
        return false;
    }
    bdBool isEncrypted = encrypt;
    if (!isEncrypted)
    {
        bdUInt32 sizePrefix = messageSize + 1;
        message->setHeaderSize(5u);
        writePtr = message->getHeaderStart();
        bdUInt offset = 0;

        bdBool ok = bdBytePacker::appendBasicType<bdUInt>(writePtr, 5u, 0, &offset, &sizePrefix);
        ok = ok == bdBytePacker::appendBasicType<bdBool>(writePtr, 5u, offset, &offset, &isEncrypted);
        if (ok)
        {
            bdPendingBufferTransfer tx(&bdTaskByteBufferRef(*message), messageSize + 5);
            m_outgoingBuffers.enqueue(&tx);
        }
        pump();
        return ok;
    }
    bdHMacSHA1 hmac(m_sessionKey, sizeof(m_sessionKey));
    bdUInt signatureSize = 4;
    bdByte8 blockSize = 8;
    bdUInt cypherSize = ~(blockSize - 1) & (blockSize - 1 + messageSize + 4);
    bdAssert((signatureSize + message->getSize() + message->getPaddingSize()) >= cypherSize, "bdTaskBuffer allocation too small");
    bdUInt32 sizePrefix = cypherSize + 5;
    message->setHeaderSize(13u);
    if (message->getHeaderSize() != 13)
    {
        bdAssert(false, "Memory size for encrypted header invalid");
        return false;
    }
    bdMemset(initialVector, 0, sizeof(initialVector));
    bdCryptoUtils::calculateInitialVector(m_messageCount, initialVector);
    writePtr = message->getHeaderStart();
    bdUInt offset = 0;

    bdBool ok = bdBytePacker::appendBasicType<bdUInt>(writePtr, 13, 0, &offset, &sizePrefix);
    ok = ok == bdBytePacker::appendBasicType<bdBool>(writePtr, 13, offset, &offset, &isEncrypted);
    ok = ok == bdBytePacker::appendBasicType<bdUInt>(writePtr, 13, offset, &offset, &m_messageCount);
    bdAssert(message->getData() == (writePtr + offset + signatureSize), "Remote task serialization error");
    writePtr += offset;
    offset = 0;
    ok = ok == bdBytePacker::appendBuffer(writePtr, cypherSize, offset, &offset, signature, signatureSize);
    bdUInt payloadOffset = offset + 1;
    offset += messageSize;
    bdUInt padding = cypherSize - (messageSize + signatureSize);
    for (bdUInt i = 0; i < padding; ++i)
    {
        bdByte8 cnt = m_messageCount;
        bdBytePacker::appendBasicType<bdByte8>(writePtr, cypherSize, offset, &offset, &cnt);
    }
    hmac.process(&writePtr[payloadOffset], padding + messageSize - 1);
    hmac.getData(signature, &signatureSize);
    bdBytePacker::appendBuffer(writePtr, cypherSize, 0, &offset, signature, signatureSize);
    ok = ok == m_cypher.encrypt(initialVector, writePtr, writePtr, cypherSize);

    ++m_messageCount;
    if (ok)
    {
        bdPendingBufferTransfer tx(&bdTaskByteBufferRef(*message), sizePrefix + 4);
        m_outgoingBuffers.enqueue(&tx);
    }
    pump();
    return ok;
}

bdBool bdLobbyConnection::send(bdUByte8* buffer, bdUInt bufferSize, bdBool encrypt)
{
    bdTaskByteBufferRef message(new bdTaskByteBuffer(buffer, bufferSize, false));
    return sendTask(&bdTaskByteBufferRef(message), bufferSize, encrypt);
}

void bdLobbyConnection::sendRaw(bdUByte8* buffer, const bdUInt32 bufferSize)
{
    bdByteBufferRef message(new bdByteBuffer(buffer, bufferSize, false));
    bdPendingBufferTransfer tx(&bdByteBufferRef(message), bufferSize);
    m_outgoingBuffers.enqueue(&tx);
    pump();
}

void bdLobbyConnection::sendRaw(bdByteBufferRef buffer, const bdUInt32 bufferSize)
{
    bdPendingBufferTransfer tx(&bdByteBufferRef(buffer), bufferSize);
    m_outgoingBuffers.enqueue(&tx);
    pump();
}

bdBool bdLobbyConnection::getMessageToDispatch(bdUByte8* type, bdByteBufferRef* payload)
{
    bdUByte8* readPtr;
    bdUByte8 initialVector[24];

    if (m_status == BD_DISCONNECTED)
    {
        return false;
    }
    pump();
    if (m_status == BD_CONNECTED && recvMessageData() && receivedFullMessage())
    {
        readPtr = m_recvMessage->getHeaderStart();
        bdUInt validBytes = m_messageSize - 1;
        bdUByte8 msgType = 0;
        bdUInt offset = 0;
        if (m_recvEncryptType == 1)
        {
            bdUInt32 IVseed = 0;
            bdUInt32 signature = 0;
            bdUInt plainTextSize = m_messageSize - 5;
            bdAssert(m_recvMessage->getSize() == (plainTextSize - plaintextHeaderSize), "Invalid recv'd message");
            bdBool ok = bdBytePacker::removeBasicType<bdUInt32>(readPtr, validBytes, 0, &offset, &IVseed);
            readPtr += offset;
            bdCryptoUtils::calculateInitialVector(IVseed, initialVector);
            ok = ok == m_cypher.decrypt(initialVector, readPtr, readPtr, plainTextSize);
            ok = ok == bdBytePacker::removeBasicType<bdUInt32>(readPtr, validBytes, 0, &offset, &signature);
            if (signature == 0xDEADBEEF)
            {
                ok = ok == bdBytePacker::removeBasicType<bdUByte8>(readPtr, validBytes, offset, &offset, &msgType);
                if (ok)
                {
                    *type = msgType;
                    *payload = *m_recvMessage;
                }
            }
            else
            {
                bdLogWarn("bdLobby/bdLobbyConnection", "Decryption failed.");
                return false;
            }
        }
        else
        {
            bdUInt payloadSize = m_messageSize - 2;
            bdAssert(m_recvMessage->getSize() == payloadSize, "Invalid recv'd message");
            if (bdBytePacker::removeBasicType<bdUByte8>(readPtr, validBytes, 0, &offset, &msgType))
            {
                *type = msgType;
                *payload = *m_recvMessage;
            }
        }
        m_recvState = BD_READ_INIT;
        return true;
    }
    return false;
}

bdBool bdLobbyConnection::getMessageToDispatch(bdUByte8* type, bdBitBufferRef* payload)
{
    bdByteBufferRef buffer;

    if (getMessageToDispatch(type, &buffer) && buffer.notNull())
    {
        *payload = new bdBitBuffer(buffer->getData(), buffer->getDataSize() * CHAR_BIT, true);
        return true;
    }
    return false;
}

void bdLobbyConnection::setSessionKey(const bdUByte8* const sesssionKey)
{
    bdMemcpy(m_sessionKey, m_sessionKey, sizeof(m_sessionKey));
    m_cypher.init(m_sessionKey, sizeof(m_sessionKey));
}

bdBool bdLobbyConnection::connect(bdAuthInfo* authInfo)
{
    m_status = BD_CONNECTING;
    setSessionKey(authInfo->m_sessionKey);
    bdSocketStatusCode status = m_socket.connect(&bdAddr(m_addr->getPublicAddr())); // I THINK THIS IS CORRECT
    if (status == BD_NET_SUCCESS)
    {
        m_status = BD_CONNECTED;
        callListenersConnect(true);
        return true;
    }
    else if (status == BD_NET_WOULD_BLOCK)
    {
        m_asyncConnectTimer.reset();
        m_asyncConnectTimer.start();
        return true;
    }
    else
    {
        close();
        return false;
    }
}

void bdLobbyConnection::disconnect()
{
    close();
}

void bdLobbyConnection::close()
{
    if (m_status == BD_CONNECTING)
    {
        callListenersConnect(false);
    }
    else if (m_status != BD_DISCONNECTED)
    {
        callListenersDisconnect();
    }
    m_status = BD_DISCONNECTED;
    m_socket.close();
}

bdBool bdLobbyConnection::pump()
{
    bdSocketStatusCode sockError;

    if (m_status == BD_CONNECTING)
    {
        sockError = BD_NET_SUCCESS;
        if (m_socket.isWritable(&sockError))
        {
            if (sockError == BD_NET_SUCCESS)
            {
                m_status = BD_CONNECTED;
                callListenersConnect(true);
            }
        }
        else if (sockError == BD_NET_SUCCESS)
        {
            if (m_asyncConnectTimer.getElapsedTimeInSeconds() <= 30.0)
            {
                return true;
            }
            bdLogWarn("bdLobby/bdLobbyConnection", "Failed to establish connection due to timeout");
            close();
            return false;
        }
        bdLogWarn("bdLobby/bdLobbyConnection", "Failed to establish connection due to socket error %d", sockError);
        close();
        return false;
    }
    while (m_status == BD_CONNECTED && !m_outgoingBuffers.isEmpty())
    {
        bdPendingBufferTransfer* tx = m_outgoingBuffers.peek();
        bdInt sentStatus = m_socket.send(tx->getData(), tx->getAvail());
        if (sentStatus <= 0)
        {
            switch (sentStatus)
            {
            case -11:
                bdLogWarn("bdLobby/bdLobbyConnection", "not connected to host!");
                close();
                break;
            case -10:
                bdLogWarn("bdLobby/bdLobbyConnection", "invalid handle.");
                close();
                break;
            case -7:
                bdLogInfo("bdLobby/bdLobbyConnection", "invalid address. Closing connection.");
                close();
                break;
            case -6:
            case -5:
                close();
                break;
            case -4:
            case -2:
                break;
            case -3:
                bdLogWarn("bdLobby/bdLobbyConnection", "net subsystem error!");
                close();
                break;
            case -1:
                bdLogWarn("bdLobby/bdLobbyConnection", "unknown error.");
                close();
                break;
            default:
                bdLogWarn("bdLobby/bdLobbyConnection", "unknown error.");
                close();
                break;
            }
            return m_status == BD_CONNECTED;
        }
        m_keepAliveTimer.start();
        if (!tx->updateTransfer(sentStatus))
        {
            m_outgoingBuffers.dequeue();
        }
    }
    if (m_keepAliveTimer.getElapsedTimeInSeconds() > 40.0)
    {
        bdUInt32 keepAlive = 0;
        bdInt socketResult = m_socket.send(&keepAlive, sizeof(keepAlive));
        if (socketResult < 0 && socketResult != -2)
        {
            bdLogError("bdLobby/bdLobbyConnection", "KeepAlive packet failed to send! Closing.");
            close();
            return false;
        }
        m_keepAliveTimer.start();
    }
    if (m_lastReceivedTimer.getElapsedTimeInSeconds() > 120.0)
    {
        bdLogWarn("bdLobby/bdLobbyConnection", "Connection timed out\n");
        close();
    }
    return true;
}

void bdLobbyConnection::callListenersConnect(const bdBool success)
{
    if (success)
    {
        m_keepAliveTimer.reset();
        m_keepAliveTimer.start();
        m_lastReceivedTimer.reset();
        m_lastReceivedTimer.start();
        if (m_connectionListener)
        {
            m_connectionListener->onConnect(&bdLobbyConnectionRef(this));
        }
    }
    else if (m_connectionListener)
    {
        m_connectionListener->onConnectFailed(&bdLobbyConnectionRef(this));
    }
}

void bdLobbyConnection::callListenersDisconnect()
{
    if (m_connectionListener)
    {
        m_connectionListener->onDisconnect(&bdLobbyConnectionRef(this));
    }
}

bdBool bdLobbyConnection::recvMessageData()
{
    bdInt status;

    if (m_status != BD_CONNECTED)
    {
        return false;
    }
    bdLobbyConnection::RecvState oldState = m_recvState;
    switch (oldState)
    {
    case BD_READ_INIT:
        m_recvCount = 0;
        m_recvEncryptType = 0;
        m_messageSize = 0;
        m_recvMessage = (bdTaskByteBuffer*)NULL;
        m_recvTransfer = (bdPendingBufferTransfer*)NULL;
        m_recvState = BD_READ_SIZE;
        status = recvMessageSize();
        break;
    case BD_READ_SIZE:
        status = recvMessageSize();
        break;
    case BD_READ_ENCRYPT:
        status = recvEncryptType();
        break;
    case BD_READ_MESSAGE:
        bdAssert(m_recvTransfer->getAvail() > 0, "BD_READ_MESSAGE state error");
        status = m_socket.recv(m_recvTransfer->getData(), m_recvTransfer->getAvail());
        if (status > 0)
        {
            if (!m_recvTransfer->updateTransfer(status))
            {
                m_recvState = BD_READ_COMPLETE;
            }
        }
        break;
    default:
        return true;
    }
    if (status > 0)
    {
        m_lastReceivedTimer.reset();
        m_lastReceivedTimer.start();
    }
    if (m_recvState == oldState)
    {
        if (status <= 0 && !receivedFullMessage())
        {
            switch (status)
            {
            case -11:
            case -9:
            case -8:
            case -2:
                break;
            case -6:
                bdLogWarn("bdLobby/bdLobbyConnection", "Couldn't receive message. Buffer too small?");
                break;
            case -5:
                bdLogInfo("bdLobby/bdLobbyConnection", "Received 'connection reset'. Closing connection.");
                close();
                break;
            case 0:
                m_status = BD_DISCONNECTING;
                callListenersDisconnect();
                m_status = BD_DISCONNECTED;
                break;
            default:
                bdLogWarn("bdLobby/bdLobbyConnection", "net error. status : %i", status);
                break;
            }
        }
        return m_status == BD_CONNECTED;
    }
    else
    {
        return recvMessageData();
    }
}

bdInt bdLobbyConnection::recvMessageSize()
{
    bdAssert(!m_messageSize && m_recvCount < BD_MSG_SIZE_BUFFER_SIZE, "BD_READ_SIZE state error");
    bdInt sockStatus = m_socket.recv(&m_msgSizeBuffer[m_recvCount], BD_MSG_SIZE_BUFFER_SIZE - m_recvCount);
    if (sockStatus <= 0)
    {
        return sockStatus;
    }
    m_recvCount += sockStatus;
    if (m_recvCount == BD_MSG_SIZE_BUFFER_SIZE)
    {
        bdUInt offset = 0;
        bdBytePacker::removeBasicType<bdUInt>(m_msgSizeBuffer, sizeof(m_msgSizeBuffer), 0, &offset, &m_messageSize);
        if (m_messageSize)
        {
            if (m_messageSize <= m_maxRecvMessageSize)
            {
                m_recvState = BD_READ_ENCRYPT;
            }
            else
            {
                bdLogWarn("bdLobby/bdLobbyConnection", "Message received is too large to fit in the receive buffer.");
                close();
                sockStatus = -11;
            }
        }
        else
        {
            m_recvState = BD_READ_INIT;
        }
    }
    return sockStatus;
}

bdLobbyConnection::Status bdLobbyConnection::getStatus() const
{
    return m_status;
}

bdInt bdLobbyConnection::recvEncryptType()
{
    bdInt sockStatus = m_socket.recv(&m_recvEncryptType, 1u);
    if (sockStatus <= 0)
    {
        return sockStatus;
    }
    bdAssert(m_recvMessage == BD_NULL && m_recvTransfer == BD_NULL, "BD_READ_ENCRYPT state error");
    bdUInt headerRemaining = m_recvEncryptType == 1 || m_recvEncryptType == 2 ? 9 : 1;
    bdUInt headerSize = headerRemaining + 1;
    bdAssert(m_messageSize >= headerSize, "Message body too small.");
    m_recvMessage = new bdTaskByteBuffer(m_messageSize - (headerRemaining + 1), true);
    m_recvMessage->setHeaderSize(headerRemaining + 5);
    bdUByte8* preHeaderSpace = m_recvMessage->getHeaderStart();
    bdUInt preHeaderOffset = 0;
    bdBool ok = false;
    if (bdBytePacker::appendBasicType<bdUInt>(preHeaderSpace, plaintextHeaderSize, preHeaderOffset, &preHeaderOffset, &m_messageSize))
    {
        ok = bdBytePacker::appendBasicType<bdUByte8>(preHeaderSpace, plaintextHeaderSize, preHeaderOffset, &preHeaderOffset, &m_recvEncryptType);
    }
    bdAssert(ok, "Pointer arithmetic failure");
    m_recvMessage->setHeaderSize(headerRemaining);
    m_recvTransfer = new bdPendingBufferTransfer(&bdTaskByteBufferRef(&m_recvMessage), m_messageSize - 1);
    m_recvState = BD_READ_MESSAGE;
    return sockStatus;
}

bdUInt bdLobbyConnection::getReceiveBufferSize() const
{
    return m_maxRecvMessageSize;
}

bdBool bdLobbyConnection::receivedFullMessage()
{
    return m_recvState == BD_READ_COMPLETE;
}
