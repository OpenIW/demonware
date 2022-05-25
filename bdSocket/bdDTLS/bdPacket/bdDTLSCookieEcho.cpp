// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocket.h"

void bdDTLSCookieEcho::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

bdDTLSCookieEcho::bdDTLSCookieEcho()
    : bdDTLSHeader(), m_cookie()
{
}

bdDTLSCookieEcho::bdDTLSCookieEcho(bdUInt16 vtag, bdDTLSInitAck& cookie, bdCommonAddrRef localCommonAddr, bdECCKey* ECCKey) : bdDTLSHeader(BD_DTLS_COOKIE_ECHO, vtag, 0), m_cookie(cookie)
{
    bdUByte8 key[100];
    bdSecurityID secId;
    bdUByte8 serializedCommonAddr[37];
    bdUInt keyLen;

    m_cookie.getSecID(secId);
    localCommonAddr->serialize(serializedCommonAddr);
    keyLen = 100;
    if (!ECCKey->exportKey(key, keyLen) || keyLen != 100)
    {
        bdLogWarn("dtls cookie echo", "problem with key");
    }
    bdMemcpy(m_ca, serializedCommonAddr, sizeof(m_ca));
    bdMemcpy(m_secID, &secId, sizeof(m_secID));
    bdMemcpy(m_ECCKey, key, sizeof(m_ECCKey));
}

bdDTLSCookieEcho::~bdDTLSCookieEcho()
{
    delete this;
}

bdBool bdDTLSCookieEcho::serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset)
{
    bdBool ok;

    newOffset = offset;
    ok = bdDTLSHeader::serialize(data, size, newOffset, newOffset);
    ok = ok == m_cookie.serialize(data, size, newOffset, newOffset);
    ok = ok == bdBytePacker::appendBuffer(reinterpret_cast<bdUByte8*>(data), size, newOffset, newOffset, m_ca, sizeof(m_ca));
    ok = ok == bdBytePacker::appendBuffer(reinterpret_cast<bdUByte8*>(data), size, newOffset, newOffset, m_secID, sizeof(m_secID));
    ok = ok == bdBytePacker::appendBuffer(reinterpret_cast<bdUByte8*>(data), size, newOffset, newOffset, m_ECCKey, sizeof(m_ECCKey));
    if (!ok)
    {
        newOffset = offset;
    }
    return ok;
}

bdBool bdDTLSCookieEcho::deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset)
{
    bdBool ok;

    newOffset = offset;
    ok = bdDTLSHeader::deserialize(data, size, newOffset, newOffset);
    ok = ok == m_cookie.deserialize(data, size, newOffset, newOffset);
    ok = ok == bdBytePacker::removeBuffer(reinterpret_cast<const bdUByte8*>(data), size, newOffset, newOffset, m_ca, sizeof(m_ca));
    ok = ok == bdBytePacker::removeBuffer(reinterpret_cast<const bdUByte8*>(data), size, newOffset, newOffset, m_secID, sizeof(m_secID));
    ok = ok == bdBytePacker::removeBuffer(reinterpret_cast<const bdUByte8*>(data), size, newOffset, newOffset, m_ECCKey, sizeof(m_ECCKey));
    if (!ok)
    {
        newOffset = offset;
    }
    return ok;
}

const bdDTLSInitAck& bdDTLSCookieEcho::getCookie() const
{
    return m_cookie;
}

const bdUByte8* bdDTLSCookieEcho::getECCKey() const
{
    return m_ECCKey;
}

const bdUByte8* bdDTLSCookieEcho::getCa() const
{
    return m_ca;
}

const bdUByte8* bdDTLSCookieEcho::getSecID() const
{
    return m_secID;
}
