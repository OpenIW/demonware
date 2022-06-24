// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocket.h"

bdDTLSInit::bdDTLSInit() 
    : bdDTLSHeader(), m_initTag(0), m_secID()
{
}

bdDTLSInit::bdDTLSInit(bdUInt16 initTag, bdSecurityID& secID) 
    : bdDTLSHeader(BD_DTLS_INIT, 0, 0), m_initTag(initTag), m_secID(secID)
{
}

const bdUInt16 bdDTLSInit::getInitTag() const
{
    return m_initTag;
}

void bdDTLSInit::getSecID(bdSecurityID& secID) const
{
    secID = m_secID;
}

bool bdDTLSInit::serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset)
{
    bdBool ok;

    newOffset = offset;
    ok = bdDTLSHeader::serialize(data, size, newOffset, newOffset);
    ok = ok == bdBytePacker::appendBasicType<bdUInt16>(data, size, newOffset, newOffset, m_initTag);
    ok = ok == bdBytePacker::appendBuffer(reinterpret_cast<bdUByte8*>(data), size, newOffset, newOffset, &m_secID, sizeof(bdSecurityID));
    if (!ok)
    {
        newOffset = offset;
    }
    return ok;
}

bool bdDTLSInit::deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset)
{
    bdBool ok;

    newOffset = offset;
    ok = bdDTLSHeader::deserialize(data, size, newOffset, newOffset);
    ok = ok == bdBytePacker::removeBasicType<bdUInt16>(data, size, newOffset, newOffset, m_initTag);
    ok = ok == bdBytePacker::removeBuffer(reinterpret_cast<const bdUByte8*>(data), size, newOffset, newOffset, &m_secID, sizeof(bdSecurityID));
    if (!ok)
    {
        newOffset = offset;
    }
    return ok;
}

