// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdPlatform/bdPlatform.h"

bdUInt64 bdPlatformStreamSocket::m_totalBytesSent = 0;
bdUInt64 bdPlatformStreamSocket::m_totalBytesRecvd = 0;

bdInt bdPlatformStreamSocket::create(bdBool blocking)
{
    u_long nonblocking;

    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    nonblocking = !blocking;
    if (ioctlsocket(s, FIONBIO, &nonblocking))
    {
        return SOCKET_ERROR;
    }
    return s;
}

bdSocketStatusCode bdPlatformStreamSocket::connect(bdInt handle, bdInAddr addr, bdUInt16 port)
{
    sockaddr remoteAddr;

    memset(&remoteAddr, 0, sizeof(remoteAddr));
    remoteAddr.sa_family = AF_INET;
    *(bdInAddr*)&remoteAddr.sa_data[2] = addr;
    *remoteAddr.sa_data = htons(port);
    if (::connect(handle, &remoteAddr, sizeof(remoteAddr)))
    {
        return BD_NET_SUCCESS;
    }
    switch (WSAGetLastError())
    {
    case WSAEINTR:
        return BD_NET_BLOCKING_CALL_CANCELED;
    case WSAEINVAL:
        return BD_NET_NOT_BOUND;
    case WSAEWOULDBLOCK:
        return BD_NET_WOULD_BLOCK;
    case WSAEMSGSIZE:
        return BD_NET_MSG_SIZE;
    case WSAEHOSTUNREACH:
        return BD_NET_CONNECTION_RESET;
    default:
        return BD_NET_ERROR;
    }
}

bool bdPlatformStreamSocket::close(bdInt handle)
{
    if (handle == -1)
    {
        return true;
    }
    switch (closesocket(handle))
    {
    case 0:
        return true;
    case -1:
        bdLogWarn("platform stream socket", "Failed to close socket Error %i.", WSAGetLastError());
        break;
    default:
        bdLogWarn("platform stream socket", "Failed to close socket. Unknown Error Code.");
        break;
    }
    return false;
}

bool bdPlatformStreamSocket::checkSocketException(bdInt handle)
{
    timeval zero;
    fd_set fdset;

    zero.tv_sec = 0;
    zero.tv_usec = 0;
    fdset.fd_array[0] = handle;
    fdset.fd_count = 1;

    return select(0, 0, 0, &fdset, &zero) || __WSAFDIsSet(handle, &fdset);
}

bdBool bdPlatformStreamSocket::isWritable(bdInt handle, bdSocketStatusCode& error)
{
    timeval zero;
    fd_set fdwrite;
    fd_set fderr;
    bool success;
    int i;

    zero.tv_sec = 0;
    zero.tv_usec = 0;
    fderr.fd_count = 0;
    for (i = 0; i < fderr.fd_count && fderr.fd_array[i] != handle; ++i);

    if (i == fderr.fd_count && fderr.fd_count < 64)
    {
        fderr.fd_array[i] = handle;
        ++fderr.fd_count;
    }

    error = BD_NET_SUCCESS;
    success = __WSAFDIsSet(handle, &fdwrite) && select(0, 0, &fdwrite, &fderr, &zero) != -1;
    if (__WSAFDIsSet(handle, &fderr))
    {
        switch (WSAGetLastError())
        {
        case WSAEINTR:
            error = BD_NET_BLOCKING_CALL_CANCELED;
        case WSAEINVAL:
            error = BD_NET_NOT_BOUND;
        case WSAEWOULDBLOCK:
            error = BD_NET_WOULD_BLOCK;
        case WSAEMSGSIZE:
            error = BD_NET_MSG_SIZE;
        case WSAEHOSTUNREACH:
            error = BD_NET_CONNECTION_RESET;
        default:
            error = BD_NET_ERROR;
        }
    }
    return success;
}

bdBool bdPlatformStreamSocket::getSocketAddr(bdInt handle, bdInAddr& socketAddr)
{
    sockaddr_in retrievedAddr;
    int length = 16;

    if (getsockname(handle, (sockaddr*)&retrievedAddr, &length) == -1)
    {
        return 0;
    }
    socketAddr = *(bdInAddr*)&retrievedAddr.sin_addr;
    return 1;
}

bdInt bdPlatformStreamSocket::send(bdInt handle, const void* const data, bdUInt length)
{
    if (handle == -1)
    {
        return BD_NET_INVALID_HANDLE;
    }
    bdInt sent = ::send(handle, reinterpret_cast<const char*>(data), length, 0);
    if (sent >= 0)
    {
        m_totalBytesSent += sent;
        return sent;
    }

    switch (WSAGetLastError())
    {
    case WSAEINTR:
        return BD_NET_BLOCKING_CALL_CANCELED;
    case WSAEADDRNOTAVAIL:
        return BD_NET_ADDRESS_INVALID;
    case WSAEWOULDBLOCK:
        return BD_NET_WOULD_BLOCK;
    case WSAEMSGSIZE:
        return BD_NET_MSG_SIZE;
    case WSAEHOSTUNREACH:
        return BD_NET_HOST_UNREACH;
    case WSAENOTCONN:
        return checkSocketException(handle) ? BD_NET_CONNECTION_RESET : BD_NET_NOT_CONNECTED;
    default:
        return BD_NET_ERROR;
    }
}

bdInt bdPlatformStreamSocket::receive(bdInt handle, void* const data, bdUInt length)
{
    if (handle == -1)
    {
        return BD_NET_INVALID_HANDLE;
    }
    bdInt recvd = ::recv(handle, reinterpret_cast<char*>(data), length, 0);
    if (recvd >= 0)
    {
        m_totalBytesRecvd += recvd;
        return recvd;
    }

    switch (WSAGetLastError())
    {
    case WSAEINTR:
        return BD_NET_BLOCKING_CALL_CANCELED;
    case WSAEINVAL:
        return BD_NET_NOT_BOUND;
    case WSAEMSGSIZE:
        return BD_NET_MSG_SIZE;
    case WSAEHOSTUNREACH:
        return BD_NET_HOST_UNREACH;
    case WSAENOTCONN:
        return checkSocketException(handle) ? BD_NET_CONNECTION_RESET : BD_NET_NOT_CONNECTED;
    default:
        return BD_NET_ERROR;
    }
}

bdBool bdPlatformStreamSocket::isWritable(bdInt handle)
{
    bdSocketStatusCode ignored;
    return isWritable(handle, ignored);
}
