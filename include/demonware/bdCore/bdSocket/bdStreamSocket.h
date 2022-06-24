// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdStreamSocket
{
protected:
    bdInt m_handle;
public:
    bdStreamSocket();
    ~bdStreamSocket();
    bdBool create(bdBool blocking);
    bdSocketStatusCode bind(const bdPort port);
    bdSocketStatusCode bind(bdAddr addr);
    bdSocketStatusCode connect(bdAddr addr);
    bdBool isConnected();
    bdBool isWritable(bdSocketStatusCode& error);
    bdInt send(const void* data, const bdUInt length);
    bdInt recv(void* data, const bdUInt size);
    void close();
    bdInt getHandle();
};
