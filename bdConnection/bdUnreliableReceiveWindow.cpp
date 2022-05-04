// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdConnection/bdConnection.h"

void bdUnreliableReceiveWindow::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdUnreliableReceiveWindow::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdUnreliableReceiveWindow::bdUnreliableReceiveWindow() : m_seqNumber(-1), m_recvQueue()
{
}

bdUnreliableReceiveWindow::~bdUnreliableReceiveWindow()
{
}

bdBool bdUnreliableReceiveWindow::add(bdDataChunkRef chunk)
{
    bdSequenceNumber newSeqNum(&m_seqNumber, chunk->getSequenceNumber(), 16);
    
    if (newSeqNum > &m_seqNumber)
    {
        m_recvQueue.enqueue(&chunk);
        m_seqNumber = newSeqNum;
    }
    return true;
}

bdDataChunkRef bdUnreliableReceiveWindow::getNextToRead()
{
    bdDataChunkRef chunk;

    if (!m_recvQueue.isEmpty())
    {
        chunk = m_recvQueue.peek();
        m_recvQueue.dequeue();
    }
    return chunk;
}

void bdUnreliableReceiveWindow::reset()
{
    m_seqNumber = bdSequenceNumber(-1);
    while (!m_recvQueue.isEmpty())
    {
        m_recvQueue.dequeue();
    }
}
