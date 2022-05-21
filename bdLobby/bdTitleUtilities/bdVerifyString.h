// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdVerifyString : public bdTaskResult
{
public:
    bdBool m_verified;

    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdVerifyString();
    ~bdVerifyString();
    virtual bdBool deserialize(bdByteBufferRef buffer);
    virtual bdUInt sizeOf();
};