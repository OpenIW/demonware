// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdConnection/bdConnection.h"

void bdInitChunk::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdInitChunk::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdInitChunk::bdInitChunk()
    : bdChunk(BD_CT_INIT), m_initTag(0), m_flags(BD_IC_NO_FLAGS_0), m_id(), m_theirKey(), m_windowCredit()
{
}

bdInitChunk::bdInitChunk(bdUInt32 initTag, const bdWord windowCredit)
    : bdChunk(BD_CT_INIT), m_initTag(initTag), m_flags(BD_IC_NO_FLAGS_0), m_id(), m_theirKey(), m_windowCredit(windowCredit)
{
}

bdBool bdInitChunk::deserialize(const bdUByte8* const data, const bdUInt size, bdUInt* offset)
{
    bdBool ok = true;
    bdUInt bytesRead = *offset;

    if (size - *offset <= 4)
    {
        return ok;
    }
    ok = ok == bdChunk::deserialize(data, size, &bytesRead);
    ok = ok == bdBytePacker::removeBasicType<bdUByte8>(data, size, bytesRead, &bytesRead, reinterpret_cast<bdUByte8*>(&m_flags));
    bdUInt16 length = 0;
    ok = ok == bdBytePacker::removeBasicType<bdUInt16>(data, size, bytesRead, &bytesRead, &length);
    ok = ok == bdBytePacker::removeBasicType<bdUInt32>(data, size, bytesRead, &bytesRead, &m_initTag);
    if (ok)
    {
        *offset = bytesRead;
    }
    return ok;
}

bdUInt bdInitChunk::serialize(bdUByte8* data, const bdUInt32 size)
{
    bdBool ok = false;
    bdUInt offset = bdChunk::serialize(data, size);
    bdUInt16 length = 0;
    ok = bdBytePacker::appendBasicType<bdUByte8>(data, size, offset, &offset, reinterpret_cast<bdUByte8*>(&m_flags));
    ok = ok == bdBytePacker::appendBasicType<bdUInt16>(data, size, offset, &offset, &length);
    ok = ok == bdBytePacker::appendBasicType<bdUInt32>(data, size, offset, &offset, &m_initTag);
    return offset;
}

bdUInt bdInitChunk::getSerializedSize()
{
    return serialize(NULL, NULL);
}

const bdWord bdInitChunk::getWindowCredit() const
{
    return m_windowCredit;
}

const bdUInt32 bdInitChunk::getInitTag() const
{
    return m_initTag;
}