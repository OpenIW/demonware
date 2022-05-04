// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdConnection/bdConnection.h"

void bdUnreliableSendWindow::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdUnreliableSendWindow::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdUnreliableSendWindow::bdUnreliableSendWindow() : m_seqNumber(0), m_sendQueue()
{
}

bdUnreliableSendWindow::~bdUnreliableSendWindow()
{
}

void bdUnreliableSendWindow::add(bdDataChunkRef chunk)
{
    bdAssert(chunk.notNull(), "Chunk should never be null.");
    m_sendQueue.enqueue(&chunk);
}

void bdUnreliableSendWindow::getDataToSend(bdPacket* packet)
{
    bdBool addedChunk = 1;
    bdDataChunkRef chunk;
    while (addedChunk && !m_sendQueue.isEmpty())
    {
        chunk = m_sendQueue.peek();
        bdAssert(chunk.notNull(), "Chunk should never be null.");
        chunk->setSequenceNumber(m_seqNumber);
        addedChunk = packet->addChunk(reinterpret_cast<bdChunkRef*>(&bdDataChunkRef(&chunk)));
        if (addedChunk)
        {
            ++m_seqNumber;
            m_sendQueue.dequeue();
        }
    }
}

void bdUnreliableSendWindow::reset()
{
    m_seqNumber = 0;
    while (!m_sendQueue.isEmpty())
    {
        m_sendQueue.dequeue();
    }
}
