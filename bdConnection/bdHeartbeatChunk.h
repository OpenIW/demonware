// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdHeartbeatChunk : public bdChunk
{
public:
    enum bdHeartbeatFlags : bdInt
    {
        BD_CA_NO_FLAGS_1 = 0x0,
    };
protected:
    bdHeartbeatFlags m_flags;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdHeartbeatChunk();
    virtual bdUInt serialize(bdUByte8* data, const bdUInt32 size) const;
    virtual bdBool deserialize(const bdUByte8* const data, const bdUInt size, bdUInt& offset);
    virtual bdUInt getSerializedSize();

    const bdHeartbeatFlags getFlags() const;
};

typedef bdReference<bdHeartbeatChunk> bdHeartbeatChunkRef;
