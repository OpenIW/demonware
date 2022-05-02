// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdHeartbeatAckChunk : bdChunk
{
public:
    enum bdHeartbeatAckFlags : bdInt
    {
        BD_CA_NO_FLAGS = 0x0,
    };
protected:
    bdHeartbeatAckFlags m_flags;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdHeartbeatAckChunk();
    bdUInt serialize(bdUByte8* data, const bdUInt32 size);
    bdBool deserialize(const bdUByte8* const data, const bdUInt size, bdUInt* offset);
    bdUInt getSerializedSize();
    const bdHeartbeatAckFlags getFlags() const;
};

typedef bdReference<bdHeartbeatAckChunk> bdHeartbeatAckChunkRef;
