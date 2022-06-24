// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdSocket
{
protected:
    bdInt m_handle;
public:
    void* operator new(bdUWord nbytes);
    void operator delete(void* p);
    bdSocket();
    virtual ~bdSocket();
    virtual bdBool create(const bdBool blocking, const bdBool broadcast);
    virtual bdSocketStatusCode bind(bdAddr& addr);
    virtual bdSocketStatusCode bind(const bdPort port);
    virtual bdInt sendTo(bdAddr& addr, const void* data, const bdUInt length);
    virtual bdInt receiveFrom(bdAddr& addr, void* data, const bdUInt size);
    virtual bdBool close();
    bdInt getHandle();
};
