// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdConnection/bdConnection.h"

bdCookie::bdCookie() : bdReferencable(), m_localTag(0), m_peerTag(0), m_localTieTag(0), m_peerTieTag(0)
{

}

bdCookie::bdCookie(bdUInt32 localTag, bdUInt32 peerTag, bdUInt32 localTieTag, bdUInt32 peerTieTag)
    : bdReferencable(), m_localTag(localTag), m_peerTag(peerTag), m_localTieTag(localTieTag), m_peerTieTag(peerTieTag)
{
    if (!m_secretInitialized)
    {
        bdSingleton<bdTrulyRandomImpl>::getInstance()->getRandomUByte8(m_secret, sizeof(m_secret));
        m_secretInitialized = true;
    }
}

void* bdCookie::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

void bdCookie::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

bdUInt bdCookie::serialize(bdUByte8* data, const bdUInt32 size)
{
    bdBool ok = true;
    bdUInt offset = 20;
    bdUInt32 timeTag = 0;

    AppendBasicType(ok, bdUInt32, data, size, 20, &offset, &timeTag);
    AppendBasicType(ok, bdUInt32, data, size, offset, &offset, &m_localTag);
    AppendBasicType(ok, bdUInt32, data, size, offset, &offset, &m_peerTag);
    AppendBasicType(ok, bdUInt32, data, size, offset, &offset, &m_peerTieTag);
    AppendBasicType(ok, bdUInt32, data, size, offset, &offset, &m_localTieTag);
    if (data)
    {
        bdHMacSHA1 hmac(m_secret, sizeof(m_secret));
        bdUInt hmacSize = 20;
        hmac.process(data + 20, offset - 20);
        hmac.getData(data, &hmacSize);
    }
    return offset;
}

bdBool bdCookie::deserialize(const bdUByte8* const data, const bdUInt size, bdUInt* offset)
{
    bdUInt bytesRead;
    bdUByte8 tmpHMac[20];
    const bdUByte8* cookieData;

    bdBool ok = true;
    if (size - *offset <= 20)
    {
        return false;
    }
    bytesRead = *offset;
    cookieData = &data[*offset];
    bdHMacSHA1 hmac(m_secret, sizeof(m_secret));
    bdUInt hmacSize = 20;
    hmac.process(cookieData + 20, getSerializedSize() - 20);
    hmac.getData(tmpHMac, &hmacSize);
    if (bdMemcmp(cookieData, tmpHMac, sizeof(tmpHMac)))
    {
        bdLogWarn("bdConnection/packet", "cookie failed HMac test.");
        return false;
    }
    bytesRead += 20;
    bdUInt32 timeTag = 0;
    RemoveBasicType(ok, bdUInt32, data, size, bytesRead, &bytesRead, &timeTag);
    RemoveBasicType(ok, bdUInt32, data, size, bytesRead, &bytesRead, &m_localTag);
    RemoveBasicType(ok, bdUInt32, data, size, bytesRead, &bytesRead, &m_peerTag);
    RemoveBasicType(ok, bdUInt32, data, size, bytesRead, &bytesRead, &m_peerTieTag);
    RemoveBasicType(ok, bdUInt32, data, size, bytesRead, &bytesRead, &m_localTieTag);
    if (ok)
    {
        *offset = bytesRead;
    }
    return ok;
}

bdUInt bdCookie::getSerializedSize()
{
    return serialize(NULL, NULL);
}

const bdUInt32 bdCookie::getLocalTag() const
{
    return m_localTag;
}

const bdUInt32 bdCookie::getPeerTag() const
{
    return m_peerTag;
}

const bdUInt32 bdCookie::getLocalTieTag() const
{
    return m_localTieTag;
}

const bdUInt32 bdCookie::getPeerTieTag() const
{
    return m_peerTieTag;
}
