// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocket.h"

void bdDTLSError::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

bdDTLSError::bdDTLSError() 
    : bdDTLSHeader(), m_etype(BD_DTLS_INVALID_STATE), m_secID()
{
}

bdDTLSError::bdDTLSError(bdUInt16 vtag, bdDTLSErrorType etype, const bdSecurityID* secID) 
    : bdDTLSHeader(BD_DTLS_ERROR, vtag, 0), m_etype(etype), m_secID(secID)
{
}

bdDTLSError::~bdDTLSError()
{
    delete this;
}

bdBool bdDTLSError::serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset)
{
    bdBool ok;
    bdUByte8 shortEtype = m_etype;

    *newOffset = offset;
    ok = bdDTLSHeader::serialize(data, size, *newOffset, newOffset);
    ok = ok == bdBytePacker::appendBasicType<bdUByte8>(data, size, *newOffset, newOffset, &shortEtype);
    ok = ok == bdBytePacker::appendBuffer(reinterpret_cast<bdUByte8*>(data), size, *newOffset, newOffset, &m_secID, sizeof(bdSecurityID));
    if (!ok)
    {
        *newOffset = offset;
    }
    return ok;
}

bdBool bdDTLSError::deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset)
{
    bdBool ok;
    bdUByte8 shortEtype = -1;

    *newOffset = offset;
    ok = bdDTLSHeader::deserialize(data, size, *newOffset, newOffset);
    ok = ok == bdBytePacker::removeBasicType<bdUByte8>(data, size, *newOffset, newOffset, &shortEtype);
    m_etype = static_cast<bdDTLSErrorType>(shortEtype);
    ok = ok == bdBytePacker::removeBuffer(reinterpret_cast<const bdUByte8*>(data), size, *newOffset, newOffset, &m_secID, sizeof(bdSecurityID));
    if (!ok)
    {
        *newOffset = offset;
    }
    return ok;
}

bdInt bdDTLSError::getEtype()
{
    return m_etype;
}

void bdDTLSError::getSecID(bdSecurityID* secID)
{
    *secID = m_secID;
}
