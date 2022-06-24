// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdCookieAckChunk : public bdChunk
{
public:
    enum bdCookieAckFlags : bdInt
    {
      BD_CA_NO_FLAGS_0 = 0x0,
    };
protected:
    bdCookieAckFlags m_flags;
public:
    void operator delete(void* p);
    void* operator new (bdUWord nbytes);
    bdCookieAckChunk();
    virtual bdUInt serialize(bdUByte8* data, const bdUInt32 size) const;
    virtual bdBool deserialize(const bdUByte8* const data, const bdUInt size, bdUInt& offset);
    virtual bdUInt getSerializedSize();

    const bdCookieAckFlags getFlags() const;
};
