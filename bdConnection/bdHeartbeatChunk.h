// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdHeartbeatChunk : bdChunk
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
    bdUInt serialize(bdUByte8* data, const bdUInt32 size);
    bdBool deserialize(const bdUByte8* const data, const bdUInt size, bdUInt* offset);
    bdUInt getSerializedSize();

    const bdHeartbeatFlags getFlags() const;
};

typedef bdReference<bdHeartbeatChunk> bdHeartbeatChunkRef;
