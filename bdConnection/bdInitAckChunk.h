// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdInitAckChunk : public bdChunk
{
public:
    enum bdInitAckChunkFlags : bdInt
    {
        BD_IC_NO_FLAGS = 0x0,
    };
protected:
    bdUInt32 m_initTag;
    bdInitAckChunk::bdInitAckChunkFlags m_flags;
    bdByteBufferRef m_cookieBuffer;
    bdCookieRef m_cookie;
    bdWord m_windowCredit;
    bdUInt32 m_peerTag;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdInitAckChunk();
    bdInitAckChunk(bdUInt32 initTag, bdCookieRef cookie, const bdWord windowCredit, bdUInt32 peerTag);
    virtual bdUInt serialize(bdUByte8* data, const bdUInt32 size) const;
    virtual bdBool deserialize(const bdUByte8* const data, const bdUInt size, bdUInt& offset);
    virtual bdUInt getSerializedSize();
    bdBool getCookie(bdByteBufferRef& cookie);

    const bdWord getWindowCredit() const;
    const bdUInt32 getInitTag() const;
    const bdUInt32 getPeerTag() const;
};

typedef bdReference<bdInitAckChunk> bdInitAckChunkRef;
