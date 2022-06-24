// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdProfileInfo : public bdTaskResult
{
public:
    bdUInt64 m_entityID;

    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdProfileInfo();
    ~bdProfileInfo();
    virtual bdBool deserialize(bdByteBufferRef buffer);
    virtual bdUInt sizeOf();
    virtual void serialize(bdByteBuffer& buffer);
};
