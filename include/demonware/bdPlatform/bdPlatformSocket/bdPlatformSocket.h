// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

enum bdSocketStatusCode
{
    BD_NET_SUCCESS = 0x1,
    BD_NET_CONNECTION_CLOSED = 0x0,
    BD_NET_ERROR = 0xFFFFFFFF,
    BD_NET_WOULD_BLOCK = 0xFFFFFFFE,
    BD_NET_SUBSYTEM_ERROR = 0xFFFFFFFD,
    BD_NET_ADDRESS_IN_USE = 0xFFFFFFFC,
    BD_NET_CONNECTION_RESET = 0xFFFFFFFB,
    BD_NET_MSG_SIZE = 0xFFFFFFFA,
    BD_NET_ADDRESS_INVALID = 0xFFFFFFF9,
    BD_NET_BLOCKING_CALL_CANCELED = 0xFFFFFFF8,
    BD_NET_NOT_BOUND = 0xFFFFFFF7,
    BD_NET_INVALID_HANDLE = 0xFFFFFFF6,
    BD_NET_NOT_CONNECTED = 0xFFFFFFF5,
    BD_NET_CONNECTION_REFUSED = 0xFFFFFFF4,
    BD_NET_HOST_UNREACH = 0xFFFFFFF3,
};

typedef bdInt bdDNSHandle;

class bdPlatformSocket
{
private:
    static bdInt64 m_totalBytesSent;
    static bdInt64 m_totalPacketsSent;
    static bdInt64 m_totalBytesRecvd;
    static bdInt64 m_totalPacketsRecvd;
public:
    static bdInt create(bdBool blocking, bdBool broadcast);
    static bdSocketStatusCode bind(bdInt& handle, class bdInAddr addr, bdUInt16 port);
    static bdInt sendTo(bdInt handle, class bdInAddr addr, bdUInt16 port, const void* data, bdUInt len);
    static bdInt receiveFrom(bdInt handle, class bdInAddr& addr, bdUInt16& port, void* data, bdUInt len);
    static bdBool close(bdInt& handle);
    static bdUInt getHostByName(const char* name, class bdInAddr* addresses, bdInt numAddresses);
};
