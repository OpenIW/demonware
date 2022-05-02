// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdCookieAckChunk : bdChunk
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
    bdUInt serialize(bdUByte8* data, const bdUInt32 size);
    bdBool deserialize(const bdUByte8* const data, const bdUInt size, bdUInt* offset);
    bdUInt getSerializedSize();

    const bdCookieAckFlags getFlags() const;
};