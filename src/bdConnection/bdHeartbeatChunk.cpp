// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdConnection/bdConnection.h"

void bdHeartbeatChunk::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdHeartbeatChunk::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdHeartbeatChunk::bdHeartbeatChunk() : bdChunk(BD_CT_HEARTBEAT), m_flags(BD_CA_NO_FLAGS_1)
{
}

bdUInt bdHeartbeatChunk::serialize(bdUByte8* data, const bdUInt32 size) const
{
    bdUInt offset = bdChunk::serialize(data, size);
    bdUInt16 length = 0;

    bdBool ok = bdBytePacker::appendBasicType<bdUByte8>(data, size, offset, offset, m_flags);
    ok = ok == bdBytePacker::appendBasicType<bdUInt16>(data, size, offset, offset, length);
    return offset;
}

bdBool bdHeartbeatChunk::deserialize(const bdUByte8* const data, const bdUInt size, bdUInt& offset)
{
    bdUInt bytesRead = offset;

    bdBool ok = bdChunk::deserialize(data, size, bytesRead);
    bdUByte8 flags = 0;
    ok = ok == bdBytePacker::removeBasicType<bdUByte8>(reinterpret_cast<const void*>(data), size, bytesRead, bytesRead, flags);
    m_flags = static_cast<bdHeartbeatFlags>(flags);
    bdUInt16 length = 0;
    ok = ok == bdBytePacker::removeBasicType<bdUInt16>(data, size, bytesRead, bytesRead, length);
    if (ok)
    {
        offset = bytesRead;
    }
    return ok;
}

bdUInt bdHeartbeatChunk::getSerializedSize()
{
    return serialize(NULL, NULL);
}

const bdHeartbeatChunk::bdHeartbeatFlags bdHeartbeatChunk::getFlags() const
{
    return m_flags;
}
