// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdConnection/bdConnection.h"

bdReliableSendWindow::bdMessageFrame::bdMessageFrame() : m_chunk(), m_timer(), m_sendCount(0), m_missingCount(0), m_gapAcked(0)
{
}

bdReliableSendWindow::bdMessageFrame::bdMessageFrame(bdDataChunkRef chunk) : m_chunk(chunk), m_timer(), m_sendCount(0), m_missingCount(0), m_gapAcked(0)
{
}

bdReliableSendWindow::bdMessageFrame* bdReliableSendWindow::bdMessageFrame::operator=(bdMessageFrame* other)
{
    m_chunk = other->m_chunk;
    m_timer = other->m_timer;
    m_sendCount = other->m_sendCount;
    m_missingCount = other->m_missingCount;
    m_gapAcked = other->m_gapAcked;
    return this;
}

void bdReliableSendWindow::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdReliableSendWindow::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdReliableSendWindow::bdReliableSendWindow()
    : m_lastAcked(-1), m_nextFree(0), m_frame(), m_remoteReceiveWindowCredit(15000), m_congestionWindow(2576), m_lastSent(), m_timeoutPeriod(0.3f), m_retransmitCountThreshold(3)
{
    m_lastSent.start();
}

bdBool bdReliableSendWindow::add(bdDataChunkRef chunk)
{
    bdMessageFrame* frameSlot = &m_frame[m_nextFree.getValue() % 128];
    if (!frameSlot->m_chunk)
    {
        chunk->setSequenceNumber(m_nextFree.getValue());
        frameSlot = &bdMessageFrame(bdDataChunkRef(chunk));
        ++m_nextFree;
        return true;
    }
    else
    {
        bdLogWarn("bdConnection/windows", "reliable message window is full.");
        return false;
    }
}

void bdReliableSendWindow::getDataToSend(bdPacket* packet)
{
    bdReliableSendWindow::bdMessageFrame* frameSlot;
    bdInt serializedSize;
    bdBool retransmitTimerExpired = false;
    bdBool neededFastRetransmit = false;
    bdBool haveDataToSend = false;

    for (bdSequenceNumber i = m_lastAcked + &bdSequenceNumber(1); i < &m_nextFree; ++i)
    {
        bdUInt index = i.getValue() % 128;
        if (m_frame[index].m_chunk.isNull())
        {
            continue;
        }
        frameSlot = &m_frame[index];
        bdSequenceNumber frameSeqNum(&m_lastAcked, frameSlot->m_chunk->getSequenceNumber(), 16);
        if (frameSeqNum.getValue() != i.getValue())
        {
            bdLogError("bdConnection/windows", "Window error");
        }
        if (!*frameSlot->m_chunk)
        {
            continue;
        }
        serializedSize = frameSlot->m_chunk->getSerializedSize();
        if (!frameSlot->m_sendCount)
        {
            break;
        }

        if (frameSlot->m_missingCount < m_retransmitCountThreshold)
        {
            if (frameSlot->m_timer.getElapsedTimeInSeconds() > m_timeoutPeriod)
            {
                retransmitTimerExpired = true;
                m_flightSize -= serializedSize;
            }
        }
        else
        {
            neededFastRetransmit = true;
            m_flightSize -= serializedSize;
        }
    }
    if (neededFastRetransmit)
    {
        decreaseCongestionWindow(BD_CWDR_PACKET_LOSS_DETECTED);
    }
    if (retransmitTimerExpired)
    {
        decreaseCongestionWindow(BD_CWDR_RESEND_TIMER_EXPIRED);
        m_timeoutPeriod *= 2.0f;
        if (m_timeoutPeriod > 2)
        {
            m_timeoutPeriod = 0.3f;
        }
    }
    bdBool full = false;
    for (bdSequenceNumber i = m_lastAcked + &bdSequenceNumber(1); i < &m_nextFree && !full; ++i)
    {
        frameSlot = &m_frame[i.getValue() % 128];
        if (!*frameSlot->m_chunk)
        {
            continue;
        }

        bdDataChunkRef chunk(&frameSlot->m_chunk);
        if (frameSlot->m_sendCount)
        {
            if (frameSlot->m_missingCount < m_retransmitCountThreshold)
            {
                if (frameSlot->m_timer.getElapsedTimeInSeconds() > m_timeoutPeriod && m_flightSize < m_congestionWindow)
                {
                    if (packet->addChunk(reinterpret_cast<bdChunkRef*>(&bdDataChunkRef(&chunk))))
                    {
                        bdLogInfo("bdConnection/windows", "sent retransmit (rto timeout) %u", chunk->getSequenceNumber());
                        frameSlot->m_missingCount = 0;
                        ++frameSlot->m_sendCount;
                        frameSlot->m_timer.start();
                        m_flightSize = chunk->getSerializedSize();
                        haveDataToSend = true;
                    }
                    else
                    {
                        bdLogInfo("bdConnection/windows", "packet full.");
                        full = true;
                    }
                }
            }
            else
            {
                if (packet->addChunk(reinterpret_cast<bdChunkRef*>(&bdDataChunkRef(&chunk))))
                {
                    bdLogInfo("bdConnection/windows", "sent retransmit (fast retransmit) %u", chunk->getSequenceNumber());
                    frameSlot->m_missingCount = 0;
                    ++frameSlot->m_sendCount;
                    frameSlot->m_timer.start();
                    m_flightSize = chunk->getSerializedSize();
                    haveDataToSend = true;
                }
                else
                {
                    bdLogInfo("bdConnection/windows", "packet full.");
                    full = true;
                }
            }
        }
        else
        {
            serializedSize = chunk->getSerializedSize();
            if (m_remoteReceiveWindowCredit - m_flightSize > chunk->getSerializedSize())
            {
                if (packet->addChunk(reinterpret_cast<bdChunkRef*>(&bdDataChunkRef(&chunk))))
                {
                    ++frameSlot->m_sendCount;
                    frameSlot->m_timer.start();
                    m_flightSize += serializedSize;
                    haveDataToSend = true;
                }
                else
                {
                    bdLogInfo("bdConnection/windows", "packet full.");
                    full = true;
                }
                continue;
            }
            if (m_flightSize < m_congestionWindow && serializedSize < 1288)
            {
                if (packet->addChunk(reinterpret_cast<bdChunkRef*>(&bdDataChunkRef(&chunk))))
                {
                    bdLogInfo("bdConnection/windows", "sent 1 new packet %u (rule b)", chunk->getSequenceNumber());
                    ++frameSlot->m_sendCount;
                    frameSlot->m_timer.start();
                    m_flightSize += serializedSize;
                    haveDataToSend = true;
                }
                else
                {
                    bdLogInfo("bdConnection/windows", "packet full.");
                    full = true;
                }
            }
            else
            {
                full = true;
            }
        }
    }
    if (!haveDataToSend && m_lastSent.getElapsedTimeInSeconds() > 1.0)
    {
        decreaseCongestionWindow(BD_CWDR_INACTIVE);
    }
    m_lastSent.start();
}

void bdReliableSendWindow::decreaseCongestionWindow(const bdReliableSendWindow::bdCongestionWindowDecreaseReason reason)
{
    if (reason == BD_CWDR_PACKET_LOSS_DETECTED)
    {
        m_slowStartThresh = m_congestionWindow / 2 > 2576 ? m_congestionWindow / 2 : 2576;
        m_congestionWindow = m_slowStartThresh;
        m_partialBytesAcked = 0;
    }
    else if (reason == BD_CWDR_RESEND_TIMER_EXPIRED)
    {
        m_slowStartThresh = m_congestionWindow / 2 > 2576 ? m_congestionWindow / 2 : 2576;
        m_congestionWindow = 1288;
        m_partialBytesAcked = 0;
    }
    else if (reason == BD_CWDR_INACTIVE)
    {
        m_slowStartThresh = m_congestionWindow / 2 > 5152 ? m_congestionWindow / 2 : 5152;
        m_congestionWindow = m_slowStartThresh;
        m_partialBytesAcked = 0;
        m_flightSize = 0;
    }
}

void bdReliableSendWindow::increaseCongestionWindow(const bdUWord bytesAcked)
{
    if (m_flightSize >= m_congestionWindow)
    {
        if (m_congestionWindow > m_slowStartThresh)
        {
            m_partialBytesAcked += bytesAcked;
            if (m_partialBytesAcked >= m_congestionWindow)
            {
                m_congestionWindow += 1288;
                if (m_congestionWindow >= m_partialBytesAcked)
                {
                    m_partialBytesAcked = 0;
                }
                else
                {
                    m_partialBytesAcked = m_partialBytesAcked - m_congestionWindow;
                }
            }
        }
        else if (bytesAcked <= 1288)
        {
            m_congestionWindow = m_congestionWindow + bytesAcked;
        }
        else
        {
            m_congestionWindow += 1288;
        }
    }
}

bdBool bdReliableSendWindow::handleAck(bdSAckChunkRef chunk, bdFloat32* rtt)
{
    bdSAckChunk::bdGapAckBlock* block;
    bdReliableSendWindow::bdMessageFrame* frameSlot;
    bdSequenceNumber ack(&m_lastAcked, chunk->getCumulativeAck(), 16);
    bdBool validAck = true;
    bdSequenceNumber lastSent(m_nextFree - &bdSequenceNumber(1));

    if (ack > &lastSent)
    {
        bdLogWarn("bdConnection/windows", "Acking unsent chunk.");
        validAck = false;
    }
    if (!validAck)
    {
        return validAck;
    }

    if (ack.getValue() < m_lastAcked.getValue())
    {
        return validAck;
    }

    bdUInt ackIndex = ack.getValue() % 128;
    if (*m_frame[ackIndex].m_chunk && m_frame[ackIndex].m_sendCount == 1)
    {
        *rtt = m_frame[ackIndex].m_timer.getElapsedTimeInSeconds();
    }
    else
    {
        *rtt = 0;
    }
    m_remoteReceiveWindowCredit = chunk->getWindowCredit();
    m_flightSize = 0;
    bdUInt count = 0;
    for (bdSequenceNumber i(ack + &bdSequenceNumber(1)); count < 128; ++i)
    {
        frameSlot = &m_frame[i.getValue() % 128];
        if (!*frameSlot->m_chunk)
        {
            break;
        }
        m_flightSize += frameSlot->m_sendCount * frameSlot->m_chunk->getSerializedSize();
        ++count;
    }
    bdUInt bytesAcked = 0;
    for (bdSequenceNumber i(ack + &bdSequenceNumber(1)); i < &ack; ++i)
    {
        frameSlot = &m_frame[i.getValue() % 128];
        if (frameSlot->m_chunk.notNull())
        {
            bytesAcked += frameSlot->m_chunk->getSerializedSize();
            frameSlot->m_chunk = (bdDataChunk*)NULL;
            frameSlot->m_timer.reset();
        }
        else
        {
            bdLogWarn("bdConnection/windows", "Invalid ack.");
            validAck = false;
        }
    }
    bdLinkedList<bdSAckChunk::bdGapAckBlock>* blocks = chunk->getGapList();
    bdSequenceNumber lastSeq(ack + &bdSequenceNumber(1));
    for (;;)
    {
        if (!blocks->getSize())
        {
            break;
        }
        block = blocks->getHead();
        bdSequenceNumber startSeq(ack + &bdSequenceNumber(block->m_start));
        bdSequenceNumber endSeq(ack + &bdSequenceNumber(block->m_end));
        for (bdSequenceNumber i(lastSeq); i <= &endSeq; ++i)
        {
            frameSlot = &m_frame[i.getValue() % 128];
            if (frameSlot->m_chunk.isNull())
            {
                bdLogWarn("bdConnection/windows", "Shouldn't be null!");
                validAck = false;
            }
            if (!frameSlot->m_sendCount)
            {
                bdLogWarn("bdConnection/windows", "Send count should be > 0");
                validAck = false;
            }
            if (i < &startSeq)
            {
                ++frameSlot->m_missingCount;
                if (frameSlot->m_gapAcked)
                {
                    frameSlot->m_gapAcked = false;
                    if (frameSlot->m_timer.getElapsedTimeInSeconds() == 0.0)
                    {
                        frameSlot->m_timer.start();
                    }
                }
            }
            else
            {
                frameSlot->m_gapAcked = true;
                frameSlot->m_timer.start();
                bytesAcked += frameSlot->m_chunk->getSerializedSize();
            }
        }
        lastSeq = endSeq + &bdSequenceNumber(1);
        blocks->removeHead();
    }
    increaseCongestionWindow(bytesAcked);
    m_lastAcked = ack;
    return validAck;
}

bdBool bdReliableSendWindow::isEmpty()
{
    bdReliableSendWindow::bdMessageFrame* frameSlot;
    bdBool empty = true;
    for (bdSequenceNumber i(m_lastAcked); i < &m_nextFree && empty; ++i)
    {
        frameSlot = &m_frame[i.getValue() % 128];
        if (frameSlot->m_chunk.notNull())
        {
            empty = false;
        }
    }
    return empty;
}

void bdReliableSendWindow::setTimeoutPeriod(const bdFloat32 secs)
{
    m_timeoutPeriod = secs;
}

const bdFloat32 bdReliableSendWindow::getTimeoutPeriod() const
{
    return m_timeoutPeriod;
}
