// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdConnection/bdConnection.h"

void bdShutdownAckChunk::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdShutdownAckChunk::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdShutdownAckChunk::bdShutdownAckChunk() : bdChunk(BD_CT_SHUTDOWN_ACK), m_flags(BD_SA_NO_FLAGS)
{
}

bdUInt bdShutdownAckChunk::serialize(bdUByte8* data, const bdUInt32 size) const
{
    bdUInt offset = bdChunk::serialize(data, size);
    bdBool ok = bdBytePacker::appendBasicType<bdUByte8>(data, size, offset, offset, m_flags);
    bdUInt16 length = 0;
    ok = ok == bdBytePacker::appendBasicType<bdUInt16>(data, size, offset, offset, length);
    return offset;
}

bdBool bdShutdownAckChunk::deserialize(const bdUByte8* const data, const bdUInt size, bdUInt& offset)
{
    bdUInt bytesRead = offset;

    bdBool ok = bdChunk::deserialize(data, size, bytesRead);
    bdUByte8 flags = 0;
    ok = ok == bdBytePacker::removeBasicType<bdUByte8>(data, size, bytesRead, bytesRead, flags);
    m_flags = static_cast<bdShutdownAckFlags>(flags);
    bdUInt16 length = 0;
    ok = ok == bdBytePacker::removeBasicType<bdUInt16>(data, size, bytesRead, bytesRead, length);
    if (ok)
    {
        offset = bytesRead;
    }
    return ok;
}

bdUInt bdShutdownAckChunk::getSerializedSize()
{
    return serialize(NULL, NULL);
}

const bdShutdownAckChunk::bdShutdownAckFlags bdShutdownAckChunk::getFlags() const
{
    return m_flags;
}
