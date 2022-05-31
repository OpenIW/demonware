// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdConnection/bdConnection.h"

void* bdConnectionListener::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

void bdConnectionListener::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void bdConnection::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdConnection::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdConnection::bdConnection() : bdReferencable(), m_addr(), m_addrHandle(), m_listeners(0u), m_maxTransmissionRate(0), m_status(BD_NOT_CONNECTED)
{
}

bdConnection::bdConnection(bdCommonAddrRef addr) : bdReferencable(), m_addr(addr), m_addrHandle(), m_listeners(0u), m_maxTransmissionRate(0), m_status(BD_NOT_CONNECTED)
{
}

bdConnection::~bdConnection()
{
}

void bdConnection::registerListener(bdConnectionListener* const listener)
{
    bdUInt32 index;
    if (m_listeners.findFirst(listener, index))
    {
        bdLogWarn("bdConnection/connections", "same listener registered multiple times.");
    }
    m_listeners.pushBack(listener);
}

void bdConnection::unregisterListener(bdConnectionListener* const listener)
{
    m_listeners.removeAllKeepOrder(listener);
}

void bdConnection::setAddressHandle(const bdAddrHandleRef& addr)
{
    m_addrHandle = addr;
}

void bdConnection::setTransmissionRate(const bdUInt bytesPerSecond)
{
    m_maxTransmissionRate = bytesPerSecond;
}

bdUInt bdConnection::getTransmissionRate() const
{
    return m_maxTransmissionRate;
}

const bdAddrHandleRef& bdConnection::getAddressHandle() const
{
    return m_addrHandle;
}

bdCommonAddrRef bdConnection::getAddress() const
{
    return bdCommonAddrRef(m_addr);
}

bdConnection::Status bdConnection::getStatus() const
{
    return m_status;
}

bdBool bdConnection::connect()
{
    m_status = BD_CONNECTING;
    return true;
}

void bdConnection::disconnect()
{
    m_status = BD_DISCONNECTING;
}

void bdConnection::close()
{
    m_status = BD_DISCONNECTED;
}

bdFloat32 bdConnection::GetAvgRTT()
{
    return 0.0f;
}
