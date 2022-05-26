// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdConnection/bdConnection.h"

void bdInitAckChunk::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdInitAckChunk::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdInitAckChunk::bdInitAckChunk() : bdChunk(BD_CT_INIT_ACK), m_initTag(0), m_cookieBuffer(), m_cookie(), m_windowCredit(0), m_peerTag(0)
{
}

bdInitAckChunk::bdInitAckChunk(bdUInt32 initTag, bdCookieRef cookie, const bdWord windowCredit, bdUInt32 peerTag)
    : bdChunk(BD_CT_INIT_ACK), m_initTag(initTag), m_cookieBuffer(), m_cookie(cookie), m_windowCredit(windowCredit), m_peerTag(peerTag)
{
}

bdUInt bdInitAckChunk::serialize(bdUByte8* data, const bdUInt32 size) const
{
    bdUInt offset = 0;
    if (m_cookie.isNull())
    {
        return offset;
    }
    offset = bdChunk::serialize(data, size);
    bdBool ok = bdBytePacker::appendBasicType<bdUByte8>(data, size, offset, offset, m_flags);
    bdUInt oldOffset = offset;
    offset += 2;
    ok = ok == bdBytePacker::appendBasicType<bdUInt32>(data, size, offset, offset, m_initTag);
    bdUInt cookieLength = m_cookie->serialize(data ? &data[offset] : NULL, size - offset);
    bdUInt16 length16 = static_cast<bdUInt16>(cookieLength);
    bdAssert(cookieLength == length16, "Cookie is too big to bit in 16 bits.");
    offset += cookieLength;
    ok = ok == bdBytePacker::appendBasicType<bdUInt16>(data, size, oldOffset, oldOffset, length16);
    return offset;
}

bdBool bdInitAckChunk::deserialize(const bdUByte8* const data, const bdUInt size, bdUInt& offset)
{
    bdUInt bytesRead = offset;
    bdBool ok = true;
    bdUInt16 length = 0;

    if (size - offset <= 4)
    {
        return ok;
    }
    ok = bdChunk::deserialize(data, size, bytesRead);
    bdUByte8 flags;
    ok = ok == bdBytePacker::removeBasicType<bdUByte8>(data, size, bytesRead, bytesRead, flags);
    m_flags = static_cast<bdInitAckChunkFlags>(m_flags);
    ok = ok == bdBytePacker::removeBasicType<bdUInt16>(data, size, bytesRead, bytesRead, length);
    ok = ok == bdBytePacker::removeBasicType<bdUInt32>(data, size, bytesRead, bytesRead, m_initTag);
    m_cookieBuffer = new bdByteBuffer(length, false);
    ok = ok == bdBytePacker::removeBuffer(data, size, bytesRead, bytesRead, const_cast<bdUByte8*>(m_cookieBuffer->getData()), length);
    if (ok)
    {
        offset = bytesRead;
    }
    return ok;
}

bdUInt bdInitAckChunk::getSerializedSize()
{
    return serialize(NULL, NULL);
}

bdBool bdInitAckChunk::getCookie(bdByteBufferRef& cookie)
{
    if (*m_cookieBuffer)
    {
        cookie = m_cookieBuffer;
    }
    return m_cookieBuffer.notNull();
}

const bdUInt32 bdInitAckChunk::getInitTag() const
{
    return m_initTag;
}

const bdWord bdInitAckChunk::getWindowCredit() const
{
    return m_windowCredit;
}

const bdUInt32 bdInitAckChunk::getPeerTag() const
{
    return m_peerTag;
}
