// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdConnection/bdConnection.h"

void bdShutdownChunk::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdShutdownChunk::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdShutdownChunk::bdShutdownChunk()
    : bdChunk(BD_CT_SHUTDOWN), m_flags(BD_SC_NO_FLAGS_0)
{
}

bdUInt bdShutdownChunk::serialize(bdUByte8* data, const bdUInt32 size)
{
    bdUInt offset = bdChunk::serialize(data, size);
    bdBool ok = bdBytePacker::appendBasicType<bdUByte8>(data, size, offset, &offset, reinterpret_cast<bdUByte8*>(&m_flags));
    bdUInt16 length = 0;
    ok = ok == bdBytePacker::appendBasicType<bdUInt16>(data, size, offset, &offset, &length);
    return offset;
}

bdBool bdShutdownChunk::deserialize(const bdUByte8* const data, const bdUInt size, bdUInt* offset)
{
    bdUInt bytesRead = *offset;

    bdBool ok = bdChunk::deserialize(data, size, &bytesRead);
    ok = ok == bdBytePacker::removeBasicType<bdUByte8>(data, size, bytesRead, &bytesRead, reinterpret_cast<bdUByte8*>(&m_flags));
    bdUInt16 length = 0;
    ok = ok == bdBytePacker::removeBasicType<bdUInt16>(data, size, bytesRead, &bytesRead, &length);
    if (ok)
    {
        *offset = bytesRead;
    }
    return ok;
}

bdUInt bdShutdownChunk::getSerializedSize()
{
    return serialize(NULL, NULL);
}

const bdShutdownChunk::bdShutdownFlags bdShutdownChunk::getFlags() const
{
    return m_flags;
}
