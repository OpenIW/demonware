// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdPlatformStreamSocket
{
protected:
    static bdUInt64 m_totalBytesSent;
    static bdUInt64 m_totalBytesRecvd;
public:
    static bdInt create(bdBool blocking);
    static bdSocketStatusCode connect(bdInt handle, bdInAddr addr, bdUInt16 port);
    static bdBool close(bdInt handle);
    static bdBool checkSocketException(bdInt handle);
    static bdBool isWritable(bdInt handle, bdSocketStatusCode& error);
    static bdBool getSocketAddr(bdInt handle, bdInAddr& socketAddr);
    static bdInt send(bdInt handle, const void* const data, bdUInt length);
    static bdInt receive(bdInt handle, void* const data, bdUInt length);
    static bdBool isWritable(bdInt handle);
};
