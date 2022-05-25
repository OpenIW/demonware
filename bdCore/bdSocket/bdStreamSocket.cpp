// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdCore/bdCore.h"

bdStreamSocket::bdStreamSocket()
{
    m_handle = -1;
}

bdStreamSocket::~bdStreamSocket()
{
}

bdBool bdStreamSocket::create(bdBool blocking)
{
    m_handle = bdPlatformStreamSocket::create(blocking);
    return m_handle != -1;
}

bdSocketStatusCode bdStreamSocket::bind(const bdPort port)
{
    // Not used
    return BD_NET_SUCCESS;
}

bdSocketStatusCode bdStreamSocket::bind(bdAddr addr)
{
    // Not used
    return BD_NET_SUCCESS;
}

bdSocketStatusCode bdStreamSocket::connect(bdAddr addr)
{
    return bdPlatformStreamSocket::connect(m_handle, addr.getAddress().getInAddr(), addr.getPort());
}

bdBool bdStreamSocket::isConnected()
{
    return bdPlatformStreamSocket::isWritable(m_handle);
}

bdBool bdStreamSocket::isWritable(bdSocketStatusCode& error)
{
    error = BD_NET_SUCCESS;
    return bdPlatformStreamSocket::isWritable(m_handle, error);
}

bdInt bdStreamSocket::send(const void* data, const bdUInt length)
{
    return bdPlatformStreamSocket::send(m_handle, data, length);
}

bdInt bdStreamSocket::recv(void* data, const bdUInt size)
{
    return bdPlatformStreamSocket::receive(m_handle, data, size);
}

void bdStreamSocket::close()
{
    bdBool ok = bdPlatformStreamSocket::close(m_handle);
    m_handle = -1;
    if (!ok)
    {
        bdLogError("bdSocket", "Failed to close stream socket, system resources may be leaked.");
    }
}

bdInt bdStreamSocket::getHandle()
{
    return m_handle;
}
