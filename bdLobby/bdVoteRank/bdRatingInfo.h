// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#pragma pack(8)
class bdRatingInfo : public bdTaskResult
{
public:
    bdUInt64 m_entityID;
    bdUByte8 m_rating;

    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdRatingInfo();
    virtual void serialize(bdByteBuffer* buffer);
    virtual bdBool deserialize(bdByteBufferRef buffer);
    virtual bdUInt sizeOf();
};
#pragma pop