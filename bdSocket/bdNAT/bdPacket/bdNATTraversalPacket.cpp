// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocket.h"

bdUInt bdNATTraversalPacket::serializedSize = 0;

bdNATTraversalPacket::bdNATTraversalPacket() 
    : m_type(0), m_protocolVersion(0), m_ident(0), m_addrSrc(), m_addrDest()
{
}

bdNATTraversalPacket::bdNATTraversalPacket(const bdUByte8& m_type, const bdUInt ident, const bdAddr& addrSrc, const bdAddr& addrDest) 
    : m_type(m_type), m_protocolVersion(2), m_ident(ident), m_addrSrc(addrSrc), m_addrDest(addrDest)
{
}

bdNATTraversalPacket::~bdNATTraversalPacket()
{
}

bdBool bdNATTraversalPacket::deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset)
{
    bdBool status;

    newOffset = offset;
    status = bdBytePacker::removeBasicType<bdUByte8>(data, size, newOffset, newOffset, &m_type);
    status = status == bdBytePacker::removeBasicType<bdUInt16>(data, size, newOffset, newOffset, &m_protocolVersion);
    if (m_protocolVersion < 2u)
    {
        newOffset = offset;
        return false;
    }
    status = status == bdBytePacker::removeBuffer(reinterpret_cast<const bdUByte8*>(data), size, newOffset, newOffset, m_hmac, BD_NAT_TRAV_HMAC_SIZE);
    status = status == bdBytePacker::removeBasicType<bdUInt>(data, size, newOffset, newOffset, &m_ident);
    status = status == m_addrSrc.deserialize(reinterpret_cast<const bdUByte8*>(data), size, newOffset, newOffset);
    status = status == m_addrDest.deserialize(reinterpret_cast<const bdUByte8*>(data), size, newOffset, newOffset);
    if (!status)
    {
        newOffset = offset;
        return false;
    }
    return true;
}

bdBool bdNATTraversalPacket::serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset)
{
    bdBool status;

    newOffset = offset;
    status = bdBytePacker::appendBasicType<bdUByte8>(data, size, newOffset, newOffset, &m_type);
    status = status == bdBytePacker::appendBasicType<bdUInt16>(data, size, newOffset, newOffset, &m_protocolVersion);
    status = status == bdBytePacker::appendBuffer(reinterpret_cast<bdUByte8*>(data), size, newOffset, newOffset, m_hmac, BD_NAT_TRAV_HMAC_SIZE);
    status = status == bdBytePacker::appendBasicType<bdUInt>(data, size, newOffset, newOffset, &m_ident);
    status = status == m_addrSrc.serialize(reinterpret_cast<bdUByte8*>(data), size, newOffset, newOffset);
    status = status == m_addrDest.serialize(reinterpret_cast<bdUByte8*>(data), size, newOffset, newOffset);
    if (status)
    {
        newOffset = offset;
        return false;
    }
    return true;
}

bdUByte8 bdNATTraversalPacket::getType()
{
    return m_type;
}

bdUInt16 bdNATTraversalPacket::getProtocolVersion()
{
    return m_protocolVersion;
}

bdUByte8* bdNATTraversalPacket::getHMAC()
{
    return m_hmac;
}

bdUInt bdNATTraversalPacket::getIdentifier()
{
    return m_ident;
}

bdUInt bdNATTraversalPacket::getSize()
{
    if (!serializedSize)
    {
        serialize(NULL, 65535, 0, serializedSize);
    }
    return serializedSize;
}

const bdAddr& bdNATTraversalPacket::getAddrSrc()
{
    return m_addrSrc;
}

const bdAddr& bdNATTraversalPacket::getAddrDest()
{
    return m_addrDest;
}

void bdNATTraversalPacket::setType(bdUByte8 type)
{
    m_type = type;
}

void bdNATTraversalPacket::setHMAC(const bdUByte8* hmac, const bdUInt size)
{
    bdAssert(size == BD_NAT_TRAV_HMAC_SIZE, "Trying to set HMAC of invalid length.");
    bdMemcpy(m_hmac, hmac, BD_NAT_TRAV_HMAC_SIZE);
}
