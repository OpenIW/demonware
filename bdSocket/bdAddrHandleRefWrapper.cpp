// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocket.h"

bdAddrHandleRefWrapper::bdAddrHandleRefWrapper()
    : m_handle()
{
}

bdAddrHandleRefWrapper::bdAddrHandleRefWrapper(const bdAddrHandleRefWrapper& other)
    : m_handle(other.m_handle)
{
}

bdAddrHandleRefWrapper::bdAddrHandleRefWrapper(bdAddrHandleRef handle)
    : m_handle(handle)
{
}

bdAddrHandleRefWrapper::~bdAddrHandleRefWrapper()
{
}

bdBool bdAddrHandleRefWrapper::operator==(const bdAddrHandleRefWrapper& other) const
{
    if (m_handle.notNull() && other.m_handle.notNull())
    {
        return m_handle.m_ptr == other.m_handle.m_ptr;
    }
    bdLogWarn("bdSocket/bdAddrHandle", "Unexpected Null addrHandle.");
    return false;
}

bdUInt bdAddrHandleRefWrapper::getHash(const bdAddrHandleRefWrapper& addr) const
{
    return addr.m_handle->m_endpoint.getHash();
}

bdAddrHandleRef bdAddrHandleRefWrapper::getHandle() const
{
    return m_handle;
}
