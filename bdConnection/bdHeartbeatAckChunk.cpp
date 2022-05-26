// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdConnection/bdConnection.h"

void bdHeartbeatAckChunk::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdHeartbeatAckChunk::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdHeartbeatAckChunk::bdHeartbeatAckChunk() : bdChunk(BD_CT_HEARTBEAT_ACK), m_flags(BD_CA_NO_FLAGS)
{
}

bdUInt bdHeartbeatAckChunk::serialize(bdUByte8* data, const bdUInt32 size) const
{
    bdUInt offset = bdChunk::serialize(data, size);
    bdUInt16 length = 0;

    bdBool ok = bdBytePacker::appendBasicType<bdUByte8>(data, size, offset, offset, m_flags);
    ok = ok == bdBytePacker::appendBasicType<bdUInt16>(data, size, offset, offset, length);
    return offset;
}

bdBool bdHeartbeatAckChunk::deserialize(const bdUByte8* const data, const bdUInt size, bdUInt& offset)
{
    bdUInt bytesRead = offset;

    bdBool ok = bdChunk::deserialize(data, size, bytesRead);
    bdUByte8 flags;
    ok = ok == bdBytePacker::removeBasicType<bdUByte8>(data, size, bytesRead, bytesRead, flags);
    m_flags = static_cast<bdHeartbeatAckFlags>(flags);
    bdUInt16 length = 0;
    ok = ok == bdBytePacker::removeBasicType<bdUInt16>(data, size, bytesRead, bytesRead, length);
    if (ok)
    {
        offset = bytesRead;
    }
    return ok;
}

bdUInt bdHeartbeatAckChunk::getSerializedSize()
{
    return serialize(NULL, NULL);
}

const bdHeartbeatAckChunk::bdHeartbeatAckFlags bdHeartbeatAckChunk::getFlags() const
{
    return m_flags;
}
