// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdConnection/bdConnection.h"

void* bdLoopbackConnection::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

void bdLoopbackConnection::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

bdLoopbackConnection::bdLoopbackConnection(bdCommonAddrRef addr) : bdConnection(addr), m_messages()
{
}

bdLoopbackConnection::~bdLoopbackConnection()
{
}

bdUInt bdLoopbackConnection::getDataToSend()
{
    return 0;
}

bdBool bdLoopbackConnection::getMessageToDispatch(bdMessageRef& message)
{
    if (!m_messages.isEmpty())
    {
        message = m_messages.peek();
        m_messages.dequeue();
        return true;
    }
    return false;
}

bdBool bdLoopbackConnection::receive(const bdUByte8*, const bdUInt)
{
    return true;
}

bdBool bdLoopbackConnection::send(bdMessageRef message)
{
    if (*message->getPayload())
    {
        message->getPayload()->resetReadPosition();
    }
    m_messages.enqueue(message);
    return true;
}

void bdLoopbackConnection::updateStatus()
{
    switch (m_status)
    {
    case 1:
    {
        for (bdUInt i = 0; i < m_listeners.getSize(); ++i)
        {
            m_listeners[i]->onConnect((this));
        }
        m_status = BD_CONNECTED;
        break;
    }
    case 3:
    {
        for (bdUInt i = 0; i < m_listeners.getSize(); ++i)
        {
            m_listeners[i]->onDisconnect(bdConnectionRef(this));
        }
        m_status = BD_DISCONNECTED;
        break;
    }
    default:
        break;
    }
}
