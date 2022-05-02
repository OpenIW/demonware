// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdConnection/bdConnection.h"

void bdCookieAckChunk::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdCookieAckChunk::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdCookieAckChunk::bdCookieAckChunk() : bdChunk(BD_CT_COOKIE_ACK), m_flags(bdCookieAckChunk::BD_CA_NO_FLAGS_0)
{
}

bdUInt bdCookieAckChunk::serialize(bdUByte8* data, const bdUInt32 size)
{
    bdUInt offset = bdChunk::serialize(data, size);
    bdUInt16 length = 0;
    bdBool ok = true;

    AppendBasicType(ok, bdUByte8, data, size, offset, &offset, reinterpret_cast<bdUByte8*>(&m_flags));
    AppendBasicType(ok, bdUInt16, data, size, offset, &offset, &length);

    return offset;
}

bdBool bdCookieAckChunk::deserialize(const bdUByte8* const data, const bdUInt size, bdUInt* offset)
{
    bdUInt bytesRead = *offset;
    bdBool ok = false;

    ok = bdChunk::deserialize(data, size, &bytesRead);
    RemoveBasicType(ok, bdUByte8, data, size, bytesRead, &bytesRead, reinterpret_cast<bdUByte8*>(&m_flags));
    bdUInt16 length = 0;
    RemoveBasicType(ok, bdUInt16, data, size, bytesRead, &bytesRead, &length);
    if (ok)
    {
        *offset = bytesRead;
    }
    return ok;
}

bdUInt bdCookieAckChunk::getSerializedSize()
{
    return serialize(NULL, NULL);
}

const bdCookieAckChunk::bdCookieAckFlags bdCookieAckChunk::getFlags() const
{
    return m_flags;
}
