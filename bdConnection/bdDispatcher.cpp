// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdConnection/bdConnection.h"

void bdDispatchInterceptor::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdDispatchInterceptor::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdDispatchInterceptor::~bdDispatchInterceptor()
{
}

bdDispatcher::bdDispatcher() : m_interceptors(0u)
{
}

void bdDispatcher::process(bdConnectionRef connection)
{
    bdMessageRef message;
    while (connection->getMessageToDispatch(&message))
    {
        bdReceivedMessage rmessage(&bdMessageRef(message), &bdConnectionRef(connection));
        bdBool accepted = 0;
        for (bdUInt i = 0; !accepted && i < m_interceptors.getSize(); ++i)
        {
            accepted = m_interceptors[i]->accept(&rmessage);
        }
    }
}

void bdDispatcher::registerInterceptor(bdDispatchInterceptor* const interceptor)
{
    bdUInt index;

    if (m_interceptors.findFirst(interceptor, index))
    {
        bdLogWarn("dispatcher", "Same listener registered multiple times.");
    }
    m_interceptors.pushBack(interceptor);
}

void bdDispatcher::unregisterInterceptor(bdDispatchInterceptor* const interceptor)
{
    bdUInt index;

    if (!m_interceptors.findFirst(interceptor, index))
    {
        bdLogWarn("dispatcher", "Attempt to unregister unknown listener.");
    }
    m_interceptors.removeAllKeepOrder(interceptor);
}

void bdDispatcher::reset()
{
    m_interceptors.clear();
}
