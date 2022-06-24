// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdCore/bdCore.h"

void* bdSocket::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

void bdSocket::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

bdSocket::bdSocket()
    : m_handle(-1)
{
}

bdSocket::~bdSocket()
{
}

bdBool bdSocket::create(const bdBool blocking, const bdBool broadcast)
{
    SOCKET createRes;

    if (m_handle != -1)
    {
        bdLogError("socket", "%s, already created.", __FUNCTION__);
        return false;
    }
    createRes = bdPlatformSocket::create(blocking, broadcast);
    if (createRes >= 0)
    {
        m_handle = createRes;
        return true;
    }
    return false;
}

bdSocketStatusCode bdSocket::bind(bdAddr& addr)
{
    return bdPlatformSocket::bind(m_handle, addr.getAddress().getInAddr(), addr.getPort());
}

bdSocketStatusCode bdSocket::bind(const bdPort port)
{
    bdInetAddr iaddr = bdInetAddr::Any();
    bdAddr address(iaddr, port);

    return bdSocket::bind(address);
}

bdInt bdSocket::sendTo(bdAddr& addr, const void* data, const bdUInt length)
{
    return bdPlatformSocket::sendTo(m_handle, addr.getAddress().getInAddr(), addr.getPort(), data, length);
}

bdInt bdSocket::receiveFrom(bdAddr& addr, void* data, const bdUInt size)
{
    bdInt status;
    bdInAddr inaddr;
    bdPort port;

    status = bdPlatformSocket::receiveFrom(m_handle, inaddr, port, data, size);
    if (status < 0 && status != -5)
    {
        return status;
    }
    bdInetAddr address = bdInetAddr(inaddr);
    addr.set(address, port);
    return status;
}

bdBool bdSocket::close()
{
    return bdPlatformSocket::close(m_handle);
}

bdInt bdSocket::getHandle()
{
    if (m_handle == -1)
    {
        bdLogWarn("socket", "Socket not yet created.");
    }
    return m_handle;
}
