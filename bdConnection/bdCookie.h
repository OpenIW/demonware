// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdCookie : bdReferencable
{
protected:
    static bdBool m_secretInitialized;
    static bdUByte8 m_secret[20];
    bdUInt32 m_localTag;
    bdUInt32 m_peerTag;
    bdUInt32 m_localTieTag;
    bdUInt32 m_peerTieTag;
public:
    void* operator new(bdUWord nbytes);
    void operator delete(void* p);
    bdCookie();
    bdCookie(bdUInt32 localTag, bdUInt32 peerTag, bdUInt32 localTieTag, bdUInt32 peerTieTag);
    bdUInt serialize(bdUByte8* data, const bdUInt32 size);
    bdBool deserialize(const bdUByte8* const data, const bdUInt size, bdUInt* offset);
    bdUInt getSerializedSize();

    const bdUInt32 getLocalTag() const;
    const bdUInt32 getPeerTag() const;
    const bdUInt32 getLocalTieTag() const;
    const bdUInt32 getPeerTieTag() const;
};

typedef bdReference<bdCookie> bdCookieRef;
