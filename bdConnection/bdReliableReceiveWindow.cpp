// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdConnection/bdConnection.h"

void bdReliableReceiveWindow::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdReliableReceiveWindow::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdReliableReceiveWindow::bdReliableReceiveWindow()
    : m_newest(-1), m_lastCumulative(-1), m_lastDispatched(-1), m_frame(), m_shouldAck(false), m_recvWindowCredit(15000), m_recvWindowUsedCredit(0), m_sack()
{
}

bdBool bdReliableReceiveWindow::add(bdDataChunkRef chunk)
{
    bdAssert(chunk.notNull(), "bdReliableReceiveWindow::add, null chunk.");
    bdSequenceNumber seqNumber(m_newest, chunk->getSequenceNumber(), 16);
    if (seqNumber >= m_newest && m_recvWindowUsedCredit + chunk->getSerializedSize() > m_recvWindowCredit)
    {
        bdLogInfo("bdConnection/windows", "Not enough recv window credit.");
        return false;
    }
    bdSequenceNumber other(128);
    if (seqNumber > m_lastDispatched + other)
    {
        bdLogInfo("bdConnection/windows", "Window overflow (a) - info only");
        return false;
    }
    else if (seqNumber > m_lastDispatched)
    {
        bdUInt index = seqNumber.getValue() & 127;
        if (m_frame[index].isNull())
        {
            m_recvWindowUsedCredit += chunk->getSerializedSize();
            m_frame[index] = chunk;
            if (seqNumber > m_newest)
            {
                m_newest = seqNumber;
            }
        }
        else
        {
            if (m_frame[index]->getSequenceNumber() != chunk->getSequenceNumber())
            {
                bdLogError("bdConnection/windows", "Window overflow (b) - error");
                return false;
            }
        }
    }
    calculateAck();
    return true;
}

void bdReliableReceiveWindow::calculateAck()
{
    bdSequenceNumber i;
    bdUInt index;

    for (i = (m_lastCumulative > m_lastDispatched ? m_lastCumulative : m_lastDispatched) + bdSequenceNumber(1); 
        i <= m_newest && *m_frame[i.getValue() % 128]; ++i)
    {
        m_lastCumulative = i;
    }
    if (m_lastCumulative == bdSequenceNumber(-1))
    {
        return;
    }
    m_sack = new bdSAckChunk(m_recvWindowCredit - m_recvWindowUsedCredit, bdSAckChunk::BD_SC_NO_FLAGS);
    bdSAckChunk::bdGapAckBlock block;
    for (i = m_lastCumulative + bdSequenceNumber(1); i <= m_newest; ++i)
    {
        if (*m_frame[i.getValue() % 128])
        {
            if (!block.m_start)
            {
                block.m_start = (i - m_lastCumulative).getValue();
            }
        }
        else if (block.m_start)
        {
            block.m_end = ((i - bdSequenceNumber(1)) - m_lastCumulative).getValue();
            m_sack->addGap(block);
            bdSequenceNumber dstart((m_lastCumulative + bdSequenceNumber(block.m_start)).getValue());
            bdSequenceNumber dend((m_lastCumulative + bdSequenceNumber(block.m_end)).getValue());
            for (bdSequenceNumber jj = dstart; jj < dend; ++jj)
            {
                bdInt index = jj.getValue() % 128;
                bdAssert(m_frame[index].notNull(), "Invalid gap ack.");
                bdAssert(m_frame[index]->getSequenceNumber() == static_cast<bdUInt16>(jj.getValue()), "Invalid seq num.");
            }
            block.m_start = 0;
            block.m_end = 0;
        }
    }
    if (block.m_start)
    {
        block.m_end = ((i - bdSequenceNumber(1)) - m_lastCumulative).getValue();
        m_sack->addGap(block);
    }
    m_sack->setCumulativeAck(m_lastCumulative.getValue());
}

void bdReliableReceiveWindow::getDataToSend(bdPacket& packet)
{
    if (!*m_sack)
    {
        return;
    }

    if (packet.addChunk(bdChunkRef(*m_sack)))
    {
        m_sack = reinterpret_cast<bdSAckChunk*>(NULL);
    }
    else
    {
        bdLogInfo("bdConnection/windows", "SACK chunk didn't fit in packet");
    }
}

bdDataChunkRef bdReliableReceiveWindow::getNextToRead()
{
    bdDataChunkRef dataChunk;
    bdSequenceNumber toDispatch(m_lastDispatched + bdSequenceNumber(1));
    bdUInt index = (m_lastDispatched.getValue() + 1) % 128;
    if (!*m_frame[index])
    {
        return dataChunk;
    }
    bdSequenceNumber frameSeqNumber(m_lastDispatched, m_frame[index]->getSequenceNumber(), 16);
    if (toDispatch == frameSeqNumber)
    {
        dataChunk = &m_frame[index];
        m_frame[index] = reinterpret_cast<bdDataChunkRef*>(NULL);
        m_lastDispatched = frameSeqNumber;
        m_recvWindowUsedCredit -= dataChunk->getSerializedSize();
        if (*m_sack)
        {
            m_sack->setWindowCredit(m_recvWindowCredit - m_recvWindowUsedCredit);
        }
        else
        {
            bdLogError("bdConnection/windows", "No SACK available");
        }
    }
    else
    {
        bdLogInfo("bdConnection/windows", "unexpected seq number. (%d != %d)", toDispatch.getValue(), frameSeqNumber.getValue());
    }
    return dataChunk;
}
