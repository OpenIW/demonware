// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdSessionID : public bdTaskResult
{
public:
    bdSecurityID m_sessionID;

    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdSessionID();
    bdSessionID(const bdSessionID* sessionID);
    ~bdSessionID();
    virtual void serialize(bdByteBuffer& buffer) const;
    virtual bdBool deserialize(bdByteBufferRef buffer);
    virtual bdUInt sizeOf();
};
