// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdConnection/bdConnection.h"

bdUnicastConnection::bdControlChunkStore::bdControlChunkStore(bdChunkRef chunk, bdBool lone)
    : m_chunk(&chunk), m_lone(lone)
{
}

bdUnicastConnection::bdControlChunkStore::bdControlChunkStore(bdUnicastConnection::bdControlChunkStore* other)
    : m_chunk(&other->m_chunk), m_lone(other->m_lone)
{
}

void bdUnicastConnection::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdUnicastConnection::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdUnicastConnection::bdUnicastConnection(bdUnicastConnection* other)
{
}

bdUnicastConnection::bdUnicastConnection(bdAddressMap* addrMap)
    : bdConnection(), m_stats(), m_reliableSendWindow(NULL), m_reliableRecvWindow(NULL), m_unreliableSendWindow(), m_unreliableReceiveWindow(), m_outQueue(), m_sendTimer(), m_receiveTimer(),
    m_state(BD_UC_CLOSED), m_initTimer(), m_cookieTimer(), m_shutdownTimer(), m_shutdownGuard(), m_initAckChunk(), m_addrMap(addrMap)
{
    if (!m_addrMap)
    {
        bdLogError("bdConnection/connections", "Addr map pointer not initialised.");
    }
    reset();
}

bdUnicastConnection::bdUnicastConnection(bdCommonAddrRef dest, bdAddressMap* addrMap)
    : bdConnection(&bdCommonAddrRef(&dest)), m_stats(), m_reliableSendWindow(NULL), m_reliableRecvWindow(NULL), m_unreliableSendWindow(), m_unreliableReceiveWindow(), m_outQueue(), m_sendTimer(), 
    m_receiveTimer(), m_state(BD_UC_CLOSED), m_initTimer(), m_cookieTimer(), m_shutdownTimer(), m_shutdownGuard(), m_initAckChunk(), m_addrMap(addrMap)
{
    if (!m_addrMap)
    {
        bdLogError("bdConnection/connections", "Addr map pointer not initialised.");
    }
    reset();
}

bdUnicastConnection::~bdUnicastConnection()
{
}

bdBool bdUnicastConnection::connect()
{
    bdBool connecting = false;

    if (m_state == BD_UC_CLOSED)
    {
        if (bdConnection::connect())
        {
            connecting = sendInit();
        }
        if (connecting)
        {
            m_state = BD_UC_COOKIE_WAIT;
        }
    }
    if (!connecting)
    {
        bdLogWarn("bdConnection/connections", "failed to connect!");
    }
    return connecting;
}

void bdUnicastConnection::disconnect()
{
    if (m_state > BD_UC_CLOSED)
    {
        if (m_state <= BD_UC_COOKIE_ECHOED)
        {
            close();
        }
        else if (m_state == BD_UC_ESTABLISHED)
        {
            m_state = BD_UC_SHUTDOWN_PENDING;
            m_shutdownGuard.start();
        }
    }
}

bdBool bdUnicastConnection::send(bdReference<bdMessage> message, bdBool reliable)
{
    bdBool sent = false;
    if (m_state != BD_UC_ESTABLISHED)
    {
        bdLogWarn("bdConnection/connections", "connection not established.");
        return sent;
    }
    bdUInt payLoadSize = 0;
    if (message->hasPayload())
    {
        payLoadSize = message->getPayload()->getDataSize();
    }
    if (message->hasUnencryptedPayload())
    {
        payLoadSize += message->getUnencryptedPayload()->getDataSize();
    }
    if (payLoadSize > BD_MAX_MESSAGE_SIZE)
    {
        bdLogWarn("bdConnection/connections", "Message size > BD_MAX_MESSAGE_SIZE (%u > %u).", payLoadSize, BD_MAX_MESSAGE_SIZE);
        return sent;
    }
    bdDataChunkRef chunk(new bdDataChunk(&bdMessageRef(&message), static_cast<bdDataChunk::bdDataFlags>(!reliable)));
    if (reliable)
    {
        if (!m_reliableSendWindow)
        {
            m_reliableSendWindow = new bdReliableSendWindow();
        }
        sent = m_reliableSendWindow->add(&bdDataChunkRef(&chunk));
        if (!sent)
        {
            bdLogWarn("bdConnection/connections", "Failed to add message to reliable send window.");
        }
    }
    else
    {
        m_unreliableSendWindow.add(&bdDataChunkRef(&chunk));
        sent = true;
    }
    return sent;
}

bdConnection::Status bdUnicastConnection::getStatus()
{
    switch (m_state)
    {
    case BD_UC_CLOSED:
        return BD_DISCONNECTED;
    case BD_UC_COOKIE_ECHOED:
        return BD_CONNECTING;
    case BD_UC_ESTABLISHED:
        return BD_CONNECTED;
    case BD_UC_SHUTDOWN_ACK_SENT:
        return BD_DISCONNECTING;
    default:
        bdLogWarn("bdConnection/connections", "Unknown state");
        return BD_DISCONNECTED;
    }
}

void bdUnicastConnection::close()
{
    if (m_state)
    {
        if (m_state >= BD_UC_ESTABLISHED)
        {
            callListenersDisconnect();
        }
        else
        {
            callListenersConnect(false);
        }
    }
    m_state = BD_UC_CLOSED;
}

bdConnectionStatistics* bdUnicastConnection::getStats()
{
    return &m_stats;
}

bdFloat32 bdUnicastConnection::GetAvgRTT()
{
    return getStats()->getAvgRTT();
}

bdBool bdUnicastConnection::receive(bdUByte8* buffer, const bdUInt bufferSize)
{
    if (m_shutdownGuard.getElapsedTimeInSeconds() > 5.0)
    {
        close();
        return false;
    }
    if (!bufferSize)
    {
        bdLogWarn("bdConnection/connections", "Received zero size packet passed.");
        return false;
    }
    bdBool dataHandled = false;
    bdBool handled = false;
    bdPacket packet;
    bdBool valid = packet.deserialize(buffer, bufferSize);
    m_stats.addBytesRecv(bufferSize + 28);
    if (valid)
    {
        m_stats.addPacketsRecv(1);
        m_stats.addPacketSizeRecv(bufferSize);
        bdChunkRef chunk;
        packet.getNextChunk(&chunk);
        if (*chunk)
        {
            if (chunk->isControl())
            {
                switch (chunk->getType())
                {
                case BD_CT_COOKIE_ECHO:
                    handled = handleCookieEcho(&chunk, packet.getVerificationTag());
                    break;
                case BD_CT_COOKIE_ACK:
                    handled = handleCookieAck(&chunk, packet.getVerificationTag());
                    break;
                case BD_CT_INIT:
                    handled = handleInit(&chunk);
                    break;
                case BD_CT_INIT_ACK:
                    handled = handleInitAck(&chunk, packet.getVerificationTag());
                    break;
                default:
                    break;
                }
                if (handled)
                {
                    chunk = (bdChunk*)NULL;
                }
            }
        }
        if (m_localTag == packet.getVerificationTag())
        {
            do
            {
                if (*chunk)
                {
                    switch (chunk->getType())
                    {
                    case BD_CT_DATA:
                        handled = handleData(&chunk);
                        dataHandled = handled || dataHandled;
                        break;
                    case BD_CT_SACK:
                        handled = handleSAck(&chunk);
                        break;
                    case BD_CT_HEARTBEAT:
                        handled = handleHeartbeat(&chunk);
                        break;
                    case BD_CT_HEARTBEAT_ACK:
                        handled = handleHeartbeatAck(&chunk);
                        break;
                    case BD_CT_SHUTDOWN:
                        handled = handleShutdown(&chunk);
                        break;
                    case BD_CT_SHUTDOWN_ACK:
                        handled = handleShutdownAck(&chunk);
                        break;
                    case BD_CT_SHUTDOWN_COMPLETE:
                        handled = handleShutdownComplete(&chunk);
                        break;
                    default:
                        break;
                    }
                }
            } while (packet.getNextChunk(&chunk));
        }
        if (handled)
        {
            m_receiveTimer.start();
        }
    }
    else
    {
        bdLogWarn("bdConnection/connections", "bdUnicastConnection::receive(): Invalid packet received.");
    }
    if (m_state == BD_UC_SHUTDOWN_SENT && dataHandled && !sendShutdown())
    {
        close();
    }
}

bdUInt bdUnicastConnection::getDataToSend(bdUByte8* const buffer, const bdUInt bufferSize)
{
    bdUnicastConnection::bdControlChunkStore* chunkStore;
    bdUInt32 tagToUse;

    bdPacket packet(m_peerTag, bufferSize);
    if (m_shutdownGuard.getElapsedTimeInSeconds() > 5.0)
    {
        close();
        return false;
    }
    if (m_outQueue.isEmpty())
    {
        if (m_initTimer.getElapsedTimeInSeconds() <= 0.5)
        {
            if (m_cookieTimer.getElapsedTimeInSeconds() <= 0.5)
            {
                if (m_shutdownTimer.getElapsedTimeInSeconds() > 0.5)
                {
                    if (m_state - BD_UC_SHUTDOWN_RECEIVED < BD_UC_COOKIE_ECHOED)
                    {
                        if (!sendShutdownAck())
                        {
                            close();
                        }
                    }
                    else if (m_state == BD_UC_SHUTDOWN_SENT && !sendShutdown())
                    {
                        close();
                    }
                }
            }
            else
            {
                if (!sendCookieEcho(&bdInitAckChunkRef(&m_initAckChunk)))
                {
                    close();
                }
            }
        }
        else if (!sendInit())
        {
            close();
        }
    }
    while (!m_outQueue.isEmpty())
    {
        chunkStore = m_outQueue.peek();
        if (chunkStore->m_lone)
        {
            bdChunkRef chunk;
            while (packet.getNextChunk(&chunk))
            {
                m_outQueue.enqueue(&bdControlChunkStore(&bdChunkRef(&chunk), false));
            }
            tagToUse = m_peerTag;
            if (chunkStore->m_chunk->getType() == BD_CT_INIT_ACK)
            {
                tagToUse = bdInitAckChunkRef(reference_cast<bdInitAckChunk, bdChunk>(chunkStore->m_chunk))->getPeerTag();
            }
            bdPacket lpacket(tagToUse, bufferSize);
            lpacket.addChunk(&bdChunkRef(&chunkStore->m_chunk));
            m_outQueue.dequeue();
            return lpacket.serialize(buffer, bufferSize);
        }
        if (!packet.addChunk(&bdChunkRef(&chunkStore->m_chunk)))
        {
            break;
        }
        m_outQueue.dequeue();
    }
    bdFloat32 lastReceived = m_receiveTimer.getElapsedTimeInSeconds();
    if ((m_state - BD_UC_ESTABLISHED) <= BD_UC_ESTABLISHED)
    {
        if (lastReceived <= 1800.0)
        {
            if (m_reliableRecvWindow)
            {
                m_reliableRecvWindow->getDataToSend(&packet);
            }
            if (m_reliableSendWindow)
            {
                m_reliableSendWindow->getDataToSend(&packet);
            }
            m_unreliableSendWindow.getDataToSend(&packet);
        }
        else
        {
            bdLogWarn("bdConnection/connections", "bdUnicastConnection: Connection timed out.");
            close();
        }
    }
    if (m_state == BD_UC_SHUTDOWN_RECEIVED && packet.isEmpty() && windowsEmpty())
    {
        if (sendShutdownAck())
        {
            m_state = BD_UC_SHUTDOWN_ACK_SENT;
        }
        else
        {
            close();
        }
    }
    else if (m_state == BD_UC_SHUTDOWN_PENDING && packet.isEmpty() && windowsEmpty())
    {
        if (sendShutdown())
        {
            m_state = BD_UC_SHUTDOWN_SENT;
        }
        else
        {
            close();
        }
    }
    if (m_state == BD_UC_ESTABLISHED && packet.isEmpty())
    {
        bdFloat32 lastSent = m_sendTimer.getElapsedTimeInSeconds();
        if (lastSent > 10.0)
        {
            bdLogInfo("bdConnection/connections", "Sending Heartbeat. Last send %.2fs ago.", lastSent);
            packet.addChunk(reference_cast<bdChunk, bdHeartbeatChunk>(bdHeartbeatChunkRef(new bdHeartbeatChunk())));
        }
    }
    bdUInt dataToSend = 0;
    if (!packet.isEmpty())
    {
        dataToSend = packet.serialize(buffer, bufferSize);
        if (dataToSend > bufferSize)
        {
            bdLogError("bdConnection/connections", "buffer overflow!");
        }
        m_sendTimer.start();
    }
    if (dataToSend)
    {
        m_stats.addBytesSent(dataToSend + 28);
        m_stats.addPacketsSent(1);
        m_stats.addPacketSizeSent(dataToSend);
    }
    return dataToSend;
}

bdBool bdUnicastConnection::getMessageToDispatch(bdMessageRef* message)
{
    bdBool messageGot = false;
    bdDataChunkRef chunk;
    bdMessageRef newMessage;

    if (m_reliableRecvWindow)
    {
        chunk = m_reliableRecvWindow->getNextToRead();
        if (*chunk)
        {
            newMessage = chunk->getMessage();
            messageGot = true;
        }
    }
    if (!messageGot)
    {
        chunk = m_unreliableReceiveWindow.getNextToRead();
        if (*chunk)
        {
            newMessage = chunk->getMessage();
            messageGot = true;
        }
    }
    if (messageGot)
    {
        message = &newMessage;
    }
    return messageGot;
}

bdBool bdUnicastConnection::handleData(bdReference<bdChunk>* chunk)
{
    bdDataChunkRef dchunk(reference_cast<bdDataChunk, bdChunk>(&bdChunkRef(chunk)));
    if ((dchunk->getFlags() & 1) != 0)
    {
        return m_unreliableReceiveWindow.add(&bdDataChunkRef(&dchunk));
    }
    else
    {
        if (!m_reliableRecvWindow)
        {
            m_reliableRecvWindow = new bdReliableReceiveWindow();
        }
        if (!m_reliableRecvWindow->add(&bdDataChunkRef(&dchunk)))
        {
            bdLogWarn("bdConnection/connections", "receive window full.");
            return false;
        }
        return true;
    }
}

bdBool bdUnicastConnection::handleSAck(bdReference<bdChunk>* chunk)
{
    bdFloat32 rtt;
    bdFloat32 rto;
    bdSAckChunkRef schunk(reference_cast<bdSAckChunk, bdChunk>(&bdChunkRef(chunk)));
    if (!m_reliableSendWindow)
    {
        bdLogWarn("bdConnection/connections", "invalid stream id.");
        return false;
    }
    if (!m_reliableSendWindow->handleAck(&bdSAckChunkRef(&schunk), &rtt))
    {
        bdLogWarn("bdConnection/connections", "Unable to handle ack. disconnecting connection.");
        close();
        return false;
    }
    if (rtt > 0.0)
    {
        if (m_smoothedRTT == 0.0f && m_RTTVariation == 0.0f)
        {
            m_smoothedRTT = rtt;
            m_RTTVariation = rtt / 2.0f;
        }
        else
        {
            m_RTTVariation = (0.75f * m_RTTVariation) + (0.25f * (m_smoothedRTT <= rtt ? rtt - m_smoothedRTT : m_smoothedRTT - rtt));
            m_smoothedRTT = (0.875f * m_smoothedRTT) + (0.125f * rtt);
        }
        m_stats.setLastRTT(m_smoothedRTT);
        if (rto >= 0.02f)
        {
            if (rto > 2.0f)
            {
                rto = 2.0f;
            }
        }
        else
        {
            rto = 0.02f;
        }
        m_reliableSendWindow->setTimeoutPeriod(rto);
    }
    return true;
}

bdBool bdUnicastConnection::handleInit(bdReference<bdChunk>* chunk)
{
    bdInitChunkRef ichunk(reference_cast<bdInitChunk, bdChunk>(&bdChunkRef(chunk)));
    bdSecurityID id;

    if (!m_addrMap)
    {
        bdLogError("bdConnection/connections", "Addr map pointer not initialised.");
        return false;
    }
    m_addrMap->addrToCommonAddr(&m_addrHandle, &m_addr, &id);
    if (ichunk->getInitTag())
    {
        bdLogInfo("bdConnection/connections", "uc::handling init: m_localTag: %X", ichunk->getInitTag());
        return sendInitAck(&bdInitChunkRef(&ichunk));
    }
    else
    {
        bdLogWarn("bdConnection/connections", "handleInit: invalid init tag (%X)", ichunk->getInitTag());
        return false;
    }
}

bdBool bdUnicastConnection::handleInitAck(bdReference<bdChunk>* chunk, const bdUInt vtag)
{
    if (m_state != BD_UC_COOKIE_WAIT)
    {
        return false;
    }
    if (vtag != m_localTag)
    {
        bdLogWarn("bdConnection/connections", "Invalid verification tag on init ack.");
        return false;
    }

    bdInitAckChunkRef iachunk(reference_cast<bdInitAckChunk, bdChunk>(&bdChunkRef(chunk)));
    m_peerTag = iachunk->getInitTag();
    if (!m_peerTag)
    {
        bdLogWarn("bdConnection/connections", "handleInitAck: invalid init tag (%X)", m_peerTag);
        return false;
    }
    bdLogInfo("bdConnection/connections", "uc::handling init ack: m_localTag/m_peerTag: %X/%X", m_localTag, m_peerTag);
    if (!sendCookieEcho(&bdInitAckChunkRef(&iachunk)))
    {
        return false;
    }
    m_initTimer.reset();
    m_state = BD_UC_COOKIE_ECHOED;
    return true;
}

bdBool bdUnicastConnection::handleCookieEcho(bdReference<bdChunk>* chunk, const bdUInt vtag)
{
    bdCookieEchoChunkRef cchunk(reference_cast<bdCookieEchoChunk, bdChunk>(&bdChunkRef(chunk)));
    bdCookieRef cookie;
    if (!cchunk->getCookie(&cookie))
    {
        return false;
    }
    bdUInt32 peerTieTag = cookie->getPeerTieTag();
    bdUInt32 localTieTag = cookie->getLocalTieTag();
    bdUInt32 peerTag = cookie ->getPeerTag();
    bdUInt32 localTag = cookie->getLocalTag();
    if (!m_state)
    {
        bdSecurityID id;
        if (!m_addrMap)
        {
            bdLogError("bdConnection/connections", "Addr map pointer not initialised.");
            return false;
        }
        m_addrMap->addrToCommonAddr(&m_addrHandle, &m_addr, &id);
        m_localTag = localTag;
        m_peerTag = peerTag;
        bdLogInfo("bdConnection/connections", "uc::handling cookie echo: m_localTag/m_peerTag: %X/%X", m_localTag, m_peerTag);
        if (sendCookieAck())
        {
            m_state = BD_UC_ESTABLISHED;
            callListenersConnect(true);
            return true;
        }
        return false;
    }
    if (m_localTag == localTag || m_peerTag == peerTag || m_localTag != localTieTag || m_peerTag != peerTieTag)
    {
        if ((m_localTag != localTag || m_peerTag == peerTag) && (m_localTag != localTag || peerTieTag))
        {
            if (m_localTag == localTag || m_peerTag != peerTag)
            {
                if (m_localTag == localTag && m_peerTag == peerTag)
                {
                    if (m_state != BD_UC_ESTABLISHED)
                    {
                        m_state = BD_UC_ESTABLISHED;
                        callListenersConnect(true);
                    }
                    m_initTimer.reset();
                    m_cookieTimer.reset();
                    return sendCookieAck();
                }
                else
                {
                    return true;
                }
            }
        }
        else
        {
            if (m_state != BD_UC_ESTABLISHED)
            {
                m_state = BD_UC_ESTABLISHED;
                callListenersConnect(true);
            }
            m_peerTag = peerTag;
            m_initTimer.reset();
            m_cookieTimer.reset();
            return sendCookieAck();
        }
    }
    else
    {
        reset();
        if (sendCookieAck())
        {
            bdLogInfo("bdConnection/connections", "uc::handling cookie echo: m_localTag/m_peerTag: %X/%X", m_localTag, m_peerTag);
            bdSecurityID id;
            if (!m_addrMap)
            {
                bdLogError("bdConnection/connections", "Addr map pointer not initialised.");
                return false;
            }
            m_addrMap->addrToCommonAddr(&m_addrHandle, &m_addr, &id);
            m_localTag = localTag;
            m_peerTag = peerTag;
            m_state = BD_UC_ESTABLISHED;
            callListenersReconnect();
            return true;
        }
        return false;
    }
}

bdBool bdUnicastConnection::handleCookieAck(bdReference<bdChunk>* chunk, const bdUInt vtag)
{
    if (vtag != m_localTag)
    {
        bdLogWarn("bdConnection/connections", "Invalid verification tag on cookie ack. (%X)", vtag);
        return false;
    }
    if (m_state != BD_UC_COOKIE_ECHOED)
    {
        bdLogWarn("bdConnection/connections", "Cookie ack received in invalid state: %u", m_state);
        return false;
    }

    bdLogInfo("bdConnection/connections", "uc::handling cookie ack: m_localTag/m_peerTag: %X/%X", m_localTag, m_peerTag);
    m_state = BD_UC_ESTABLISHED;
    callListenersConnect(true);
    m_cookieTimer.reset();
    return true;
}

bdBool bdUnicastConnection::handleHeartbeat(bdReference<bdChunk>* chunk)
{
    m_receiveTimer.start();
    return sendHeartbeatAck(reference_cast<bdInitChunk, bdChunk>(&bdChunkRef(chunk)));
}

bdBool bdUnicastConnection::handleHeartbeatAck(bdReference<bdChunk>*)
{
    m_receiveTimer.start();
    return true;
}

bdBool bdUnicastConnection::handleShutdown(bdReference<bdChunk>*)
{
    bdBool shutdown;
    if (m_state == BD_UC_SHUTDOWN_SENT)
    {
        bdLogInfo("bdConnection/connections", "uc::handling shutdown (b)");
        m_state = BD_UC_SHUTDOWN_RECEIVED;
        shutdown = sendShutdownAck();
    }
    else if (m_state == BD_UC_ESTABLISHED)
    {
        bdLogInfo("bdConnection/connections", "uc::handling shutdown (a)");
        m_state = BD_UC_SHUTDOWN_RECEIVED;
        shutdown = true;
    }
    else
    {
        bdLogWarn("bdConnection/connections", "uc::handling shutdown (c) - unexpected (%u)!", m_state);
    }
    if (shutdown)
    {
        bdLogInfo("bdConnection/connections", "uc::handling shutdown.");
        m_shutdownGuard.start();
    }
    return true;
}

bdBool bdUnicastConnection::handleShutdownAck(bdReference<bdChunk>*)
{
    if (m_state == BD_UC_SHUTDOWN_ACK_SENT)
    {
        bdLogInfo("bdConnection/connections", "uc::handling shutdown ack (b)");
        sendShutdownComplete();
        close();
        m_shutdownTimer.reset();
        return true;
    }
    else if (m_state == BD_UC_SHUTDOWN_SENT)
    {
        bdLogInfo("bdConnection/connections", "uc::handling shutdown ack (a)");
        sendShutdownComplete();
        close();
        m_shutdownTimer.reset();
        return true;
    }
    else
    {
        bdLogInfo("bdConnection/connections", "uc::handling shutdown ack (c) - unexpected (%u).", m_state);
        return false;
    }
}

bdBool bdUnicastConnection::handleShutdownComplete(bdReference<bdChunk>*)
{
    if (m_state == BD_UC_SHUTDOWN_ACK_SENT)
    {
        bdLogInfo("bdConnection/connections", "uc::handling shutdown complete (a)");
        m_shutdownTimer.reset();
        close();
        return true;
    }
    else
    {
        bdLogInfo("bdConnection/connections", "uc::handling shutdown complete (b) - unexpected!");
        return false;
    }
}

bdBool bdUnicastConnection::sendInit()
{
    bdBool result = true;
    if (m_state)
    {
        result = m_state == BD_UC_COOKIE_WAIT;
    }
    bdAssert(result, "invalid state to send init from.");
    if (m_initResends++ >= 200)
    {
        return false;
    }
    m_initTimer.start();
    bdInitChunkRef chunk(new bdInitChunk(m_localTag, 15000));
    m_outQueue.enqueue(&bdControlChunkStore(reference_cast<bdChunk, bdInitChunk>(&chunk), false));
    bdLogInfo("bdConnection/connections", "uc::sending init: m_localTag: %X", m_localTag);
    return true;
}

bdBool bdUnicastConnection::sendInitAck(bdReference<bdInitChunk> chunk)
{
    bdUInt32 localTieTag = 0;
    bdUInt32 peerTieTag = 0;
    bdUInt32 localTag = 0;
    bdUInt32 peerTag = chunk->getInitTag();

    switch (m_state)
    {
    case bdUnicastConnection::BD_UC_COOKIE_WAIT:
        localTag = m_localTag;
        break;
    case bdUnicastConnection::BD_UC_COOKIE_ECHOED:
        peerTieTag = m_peerTag;
        localTieTag = m_localTag;
        localTag = m_localTag;
        break;
    case bdUnicastConnection::BD_UC_SHUTDOWN_RECEIVED:
        localTag = bdSingleton<bdTrulyRandomImpl>::getInstance()->getRandomUInt();
        peerTieTag = m_peerTag;
        localTieTag = m_localTag;
        break;
    case bdUnicastConnection::BD_UC_SHUTDOWN_ACK_SENT:
        m_peerTag = peerTag;
        localTag = m_localTag;
        break;
    default:
        bdLogWarn("bdConnection/connections", "bdUnicastConnection::sendInitAck(): Failed to send init ack.");
        return false;
    }
    bdCookieRef cookie(new bdCookie(localTag, peerTag, localTieTag, peerTieTag));
    bdInitAckChunkRef iachunk(new bdInitAckChunk(localTag, &bdCookieRef(&cookie), 15000, peerTag));
    m_outQueue.enqueue(&bdControlChunkStore(&bdChunkRef(reference_cast<bdChunk, bdInitAckChunk>(&iachunk)), true));
    bdLogInfo("bdConnection/connections", "uc::sending init ack: m_localTag/localTag/m_peerTag: %X/%X/%X", m_localTag, localTag, m_peerTag);
    return true;
}

bdBool bdUnicastConnection::sendCookieEcho(bdReference<bdInitAckChunk> chunk)
{
    m_initAckChunk = chunk;
    if (m_cookieResends++ >= 200)
    {
        return false;
    }
    bdByteBufferRef cookie;
    if (chunk->getCookie(&cookie))
    {
        bdCookieEchoChunkRef cookieEcho(new bdCookieEchoChunk(bdByteBufferRef(&cookie)));
        m_outQueue.enqueue(&bdControlChunkStore(reference_cast<bdChunk,bdCookieEchoChunk>(&cookieEcho), false));
        bdLogInfo("bdConnection/connections", "uc::sending cookie echo: m_localTag/m_peerTag: %X/%X", m_localTag, m_peerTag);
    }
    return true;
}

bdBool bdUnicastConnection::sendCookieAck()
{
    m_outQueue.enqueue(&bdControlChunkStore(&bdChunkRef(&bdChunkRef(new bdCookieAckChunk())), false));
    bdLogInfo("bdConnection/connections", "uc::sending cookie ack: m_localTag/m_peerTag: %X/%X", m_localTag, m_peerTag);
    return true;
}

bdBool bdUnicastConnection::sendHeartbeat(bdReference<bdInitChunk> chunk)
{
    m_outQueue.enqueue(&bdControlChunkStore(&bdChunkRef(&bdChunkRef(new bdHeartbeatChunk())), false));
    return true;
}

bdBool bdUnicastConnection::sendHeartbeatAck(bdReference<bdInitChunk> chunk)
{
    if (m_outQueue.isEmpty())
    {
        if (windowsEmpty())
        {
            m_outQueue.enqueue(&bdControlChunkStore(&bdChunkRef(&bdChunkRef(new bdHeartbeatAckChunk())), false));
        }
    }
    return true;
}

bdBool bdUnicastConnection::sendShutdown()
{
    if (m_shutdownResends++ >= 5)
    {
        return false;
    }
    bdLogInfo("bdConnection/connections", "uc::sending shutdown (%u/%u)", m_shutdownResends, 5);
    m_outQueue.enqueue(&bdControlChunkStore(&bdChunkRef(&bdChunkRef(new bdShutdownChunk())), false));
    m_shutdownTimer.start();
    return true;
}

bdBool bdUnicastConnection::sendShutdownAck()
{
    if (m_shutdownResends++ >= 5)
    {
        return false;
    }
    bdLogInfo("bdConnection/connections", "uc::sending shutdown ack (%u/%u)", m_shutdownResends, 5);
    m_outQueue.enqueue(&bdControlChunkStore(&bdChunkRef(&bdChunkRef(new bdShutdownAckChunk())), false));
    m_shutdownTimer.start();
    return true;
}

bdBool bdUnicastConnection::sendShutdownComplete()
{
    bdLogInfo("bdConnection/connections", "uc::sending shutdown complete");
    bdChunkRef chunk(new bdShutdownCompleteChunk());
    bdControlChunkStore item(&bdChunkRef(&chunk), false);
    m_outQueue.enqueue(&item);
    return true;
}

void bdUnicastConnection::reset()
{
    if (m_reliableRecvWindow)
    {
        delete m_reliableRecvWindow;
    }
    if (m_reliableSendWindow)
    {
        delete m_reliableSendWindow;
    }
    m_reliableRecvWindow = NULL;
    m_reliableSendWindow = NULL;
    while (!m_outQueue.isEmpty())
    {
        m_outQueue.dequeue();
    }
    m_unreliableReceiveWindow.reset();
    m_unreliableSendWindow.reset();
    m_sendTimer.start();
    m_receiveTimer.start();
    m_initResends = 0;
    m_cookieResends = 0;
    m_shutdownResends = 0;
    m_localTag = bdSingleton<bdTrulyRandomImpl>::getInstance()->getRandomUInt();
    m_peerTag = 0;
    m_smoothedRTT = 0;
    m_RTTVariation = 0;
}

void bdUnicastConnection::callListenersConnect(const bdBool success)
{
    for (bdUInt i = 0; i < m_listeners.getSize(); ++i)
    {
        if (success)
        {
            (*m_listeners[i])->onConnect(&bdConnectionRef(this));
        }
        else
        {
            (*m_listeners[i])->onConnectFailed(&bdConnectionRef(this));
        }
    }
}

void bdUnicastConnection::callListenersDisconnect()
{
    bdUInt ignored;

    bdFastArray<bdConnectionListener*> copy(&m_listeners);
    for (bdUInt i = 0; i < copy.getSize(); ++i)
    {
        if (m_listeners.findFirst(copy[i], &ignored))
        {
            (*copy[i])->onDisconnect(&bdConnectionRef(this));
        }
    }
}

void bdUnicastConnection::callListenersReconnect()
{
    for (bdUInt i = 0; i < m_listeners.getSize(); ++i)
    {
        (*m_listeners[i])->onReconnect(&bdConnectionRef(this));
    }
}

bdBool bdUnicastConnection::windowsEmpty()
{
    bdBool empty = true;
    if (m_reliableSendWindow)
    {
        if (!m_reliableSendWindow->isEmpty())
        {
            return false;
        }
    }
    return empty;
}
