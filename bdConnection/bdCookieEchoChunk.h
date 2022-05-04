// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdCookieEchoChunk : public bdChunk
{
public:
    enum bdCookieEchoFlags : bdInt
    {
      BD_CE_NO_FLAGS = 0x0,
    };
protected:
    bdCookieEchoFlags m_flags;
    bdCookieRef m_cookie;
    bdByteBufferRef m_cookieBuffer;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdCookieEchoChunk();
    bdCookieEchoChunk(bdByteBufferRef cookieBuffer);
    virtual bdUInt serialize(bdUByte8* data, const bdUInt32 size);
    virtual bdBool deserialize(const bdUByte8* const data, const bdUInt size, bdUInt* offset);
    virtual bdUInt getSerializedSize();
    bdBool getCookie(bdCookieRef* cookie);

    const bdCookieEchoFlags getFlags() const;
};

typedef bdReference<bdCookieEchoChunk> bdCookieEchoChunkRef;
