// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdConnection/bdConnection.h"

void bdCookieEchoChunk::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdCookieEchoChunk::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdCookieEchoChunk::bdCookieEchoChunk() : bdChunk(BD_CT_COOKIE_ECHO), m_flags(BD_CE_NO_FLAGS), m_cookie(), m_cookieBuffer()
{
}

bdCookieEchoChunk::bdCookieEchoChunk(bdByteBufferRef cookieBuffer) : bdChunk(BD_CT_COOKIE_ECHO), m_flags(BD_CE_NO_FLAGS), m_cookie(), m_cookieBuffer(cookieBuffer)
{
}

bdUInt bdCookieEchoChunk::serialize(bdUByte8* data, const bdUInt32 size)
{
    bdUInt16 length;

    bdUInt offset = 0;
    if (m_cookie.isNull() && m_cookieBuffer.isNull())
    {
        return offset;
    }
    offset = bdChunk::serialize(data, size);
    bdBool ok = bdBytePacker::appendBasicType<bdUByte8>(data, size, offset, &offset, reinterpret_cast<bdUByte8*>(&m_flags));
    if (m_cookieBuffer.notNull())
    {
        length = m_cookieBuffer->getSize();
        ok = ok == bdBytePacker::appendBasicType<bdUInt16>(data, size, offset, &offset, &length);
        ok = ok == bdBytePacker::appendBuffer(data, size, offset, &offset, m_cookieBuffer->getData(), length);
    }
    else
    {
        bdUInt oldOffset = offset;
        offset += 2;
        length = m_cookie->serialize(data ? &data[offset] : NULL, size - offset);
        offset += length;
        ok = ok == bdBytePacker::appendBasicType<bdUInt16>(data, size, oldOffset, &oldOffset, &length);
    }
    return offset;
}

bdBool bdCookieEchoChunk::deserialize(const bdUByte8* const data, const bdUInt size, bdUInt* offset)
{
    bdUInt bytesRead = *offset;
    bdBool ok = bdChunk::deserialize(data, size, &bytesRead);
    ok = ok == bdBytePacker::removeBasicType<bdUByte8>(data, size, bytesRead, &bytesRead, reinterpret_cast<bdUByte8*>(&m_flags));
    bdUInt16 length = 0;
    ok = ok == bdBytePacker::removeBasicType<bdUInt16>(data, size, bytesRead, &bytesRead, &length);

    m_cookie = new bdCookie();
    bdUInt cookieOffset = bytesRead;
    if (ok && m_cookie->deserialize(data, size, &bytesRead) && bytesRead - cookieOffset != length)
    {
        bdLogWarn("bdConnection/packet", "Invalid cookie echo.");
        ok = false;
    }
    if (ok)
    {
        *offset = bytesRead;
    }
    return ok;
}

bdUInt bdCookieEchoChunk::getSerializedSize()
{
    return serialize(NULL, NULL);
}

bdBool bdCookieEchoChunk::getCookie(bdCookieRef* cookie)
{
    if (*m_cookie)
    {
        cookie = &m_cookie;
    }
    return m_cookie.notNull();
}

const bdCookieEchoChunk::bdCookieEchoFlags bdCookieEchoChunk::getFlags() const
{
    return m_flags;
}
