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
    return bdBool();
}

void bdReliableReceiveWindow::calculateAck()
{
}

void bdReliableReceiveWindow::getDataToSend(bdPacket* packet)
{
}

bdChunkRef bdReliableReceiveWindow::getNextToRead()
{
    return bdChunkRef();
}
