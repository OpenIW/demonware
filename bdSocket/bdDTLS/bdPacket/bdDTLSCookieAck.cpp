// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocket.h"

void bdDTLSCookieAck::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

bdDTLSCookieAck::bdDTLSCookieAck()
    : bdDTLSHeader(), m_secID()
{
}

bdDTLSCookieAck::bdDTLSCookieAck(bdUInt16 vtag, const bdUByte8* key, const bdSecurityID secID)
    : bdDTLSHeader(BD_DTLS_COOKIE_ACK, vtag, 0), m_secID(secID)
{
    bdMemcpy(m_ECCKey, key, sizeof(m_ECCKey));
}

bdDTLSCookieAck::~bdDTLSCookieAck()
{
    delete this;
}

bdBool bdDTLSCookieAck::serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset)
{
    bdBool ok;

    *newOffset = offset;
    ok = bdDTLSHeader::serialize(data, size, *newOffset, newOffset);
    ok = ok == bdBytePacker::appendBuffer(reinterpret_cast<bdUByte8*>(data), size, *newOffset, newOffset, m_ECCKey, sizeof(m_ECCKey));
    ok = ok == bdBytePacker::appendBuffer(reinterpret_cast<bdUByte8*>(data), size, *newOffset, newOffset, &m_secID, sizeof(m_secID));
    if (!ok)
    {
        *newOffset = offset;
    }
    return ok;
}

bdBool bdDTLSCookieAck::deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset)
{
    bdBool ok;

    *newOffset = offset;
    ok = bdDTLSHeader::deserialize(data, size, *newOffset, newOffset);
    ok = ok == bdBytePacker::removeBuffer(reinterpret_cast<const bdUByte8*>(data), size, *newOffset, newOffset, m_ECCKey, sizeof(m_ECCKey));
    ok = ok == bdBytePacker::removeBuffer(reinterpret_cast<const bdUByte8*>(data), size, *newOffset, newOffset, &m_secID, sizeof(m_secID));
    if (!ok)
    {
        *newOffset = offset;
    }
    return ok;
}

bdUByte8* bdDTLSCookieAck::getECCKey()
{
    return m_ECCKey;
}

void bdDTLSCookieAck::getSecID(bdSecurityID* secID)
{
    *secID = m_secID;
}
