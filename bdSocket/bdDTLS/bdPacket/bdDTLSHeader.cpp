// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocket.h"

void bdDTLSHeader::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdDTLSHeader::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdDTLSHeader::bdDTLSHeader()
{
    m_version = 0;
    m_type = 0;
    m_vtag = 0;
    m_counter = 0;
}

bdDTLSHeader::bdDTLSHeader(bdDTLSPacketTypes type, bdUInt16 vtag, bdUInt16 counter)
{
    m_vtag = vtag;
    m_version = 2;
    m_counter = counter;
    m_type = type;
}

bdDTLSHeader::~bdDTLSHeader()
{
}

bdUByte8 bdDTLSHeader::getType()
{
    return m_type;
}

bdUByte8 bdDTLSHeader::getVersion()
{
    return m_version;
}

bdUInt16 bdDTLSHeader::getVtag()
{
    return m_vtag;
}

bdUInt16 bdDTLSHeader::getCounter()
{
    return m_counter;
}

bdBool bdDTLSHeader::serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset)
{
    bdBool ok;

    newOffset = offset;
    ok = bdBytePacker::appendBasicType<bdUByte8>(data, size, newOffset, newOffset, m_type);
    ok = ok == bdBytePacker::appendBasicType<bdUByte8>(data, size, newOffset, newOffset, m_version);
    ok = ok == bdBytePacker::appendBasicType<bdUInt16>(data, size, newOffset, newOffset, m_vtag);
    ok = ok == bdBytePacker::appendBasicType<bdUInt16>(data, size, newOffset, newOffset, m_counter);
    if (!ok)
    {
        newOffset = offset;
    }
    return ok;

}

bdBool bdDTLSHeader::deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset)
{
    bdBool ok;

    newOffset = offset;
    ok = bdBytePacker::removeBasicType<bdUByte8>(data, size, newOffset, newOffset, m_type);
    ok = ok == bdBytePacker::removeBasicType<bdUByte8>(data, size, newOffset, newOffset, m_version);
    ok = ok == bdBytePacker::removeBasicType<bdUInt16>(data, size, newOffset, newOffset, m_vtag);
    ok = ok == bdBytePacker::removeBasicType<bdUInt16>(data, size, newOffset, newOffset, m_counter);
    if (!ok)
    {
        newOffset = offset;
    }
    return ok;
}
