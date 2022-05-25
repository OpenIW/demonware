// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocket.h"

bdIPDiscoveryPacketReply::bdIPDiscoveryPacketReply()
{
    m_type = 0;
    m_protocolVersion = 0;
}

bdIPDiscoveryPacketReply::bdIPDiscoveryPacketReply(const bdAddr& addr, bdUInt16 protocolVersion)
{
    m_type = 31;
    m_protocolVersion = protocolVersion;
    m_addr = bdAddr(addr);
}

bdIPDiscoveryPacketReply::~bdIPDiscoveryPacketReply()
{
}

bdBool bdIPDiscoveryPacketReply::serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset)
{
    bdBool status;

    newOffset = offset;
    status = bdBytePacker::appendBasicType<bdUByte8>(data, size, newOffset, newOffset, &m_type);
    if (m_protocolVersion == 1)
    {
        status = status == bdBytePacker::skipBytes(reinterpret_cast<bdUByte8*>(data), size, newOffset, newOffset, 1u);
    }
    status = status == bdBytePacker::appendBasicType<bdUInt16>(data, size, newOffset, newOffset, &m_protocolVersion);
    status = status == m_addr.serialize(reinterpret_cast<bdUByte8*>(data), size, newOffset, newOffset);
    if (m_protocolVersion == 1)
    {
        status = status == bdBytePacker::skipBytes(reinterpret_cast<bdUByte8*>(data), size, newOffset, newOffset, 2u);
    }
    if (!status)
    {
        newOffset = offset;
        return false;
    }
    return true;
}

bdBool bdIPDiscoveryPacketReply::deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset)
{
    bdBool status;

    newOffset = offset;
    status = bdBytePacker::removeBasicType<bdUByte8>(data, size, newOffset, newOffset, &m_type);
    if (m_type < 30 || m_type > 39)
    {
        newOffset = offset;
        return false;
    }
    status = status == bdBytePacker::removeBasicType<bdUInt16>(data, size, newOffset, newOffset, &m_protocolVersion);
    if (m_protocolVersion != 2)
    {
        status = status == bdBytePacker::rewindBytes(reinterpret_cast<const bdUByte8*>(data), size, newOffset, newOffset, 2u);
        status = status == bdBytePacker::skipBytes(reinterpret_cast<const bdUByte8*>(data), size, newOffset, newOffset, 1u);
        status = status == bdBytePacker::removeBasicType<bdUInt16>(data, size, newOffset, newOffset, &m_protocolVersion);
        if (m_protocolVersion != 1)
        {
            newOffset = offset;
            return false;
        }
    }
    status = status == m_addr.deserialize(reinterpret_cast<const bdUByte8*>(data), size, newOffset, newOffset);
    if (m_protocolVersion == 1)
    {
        status = status == bdBytePacker::skipBytes(reinterpret_cast<const bdUByte8*>(data), size, newOffset, newOffset, 2u);
    }
    if (!status)
    {
        newOffset = offset;
        return false;
    }
    return true;
}

bdUByte8 bdIPDiscoveryPacketReply::getType()
{
    return m_type;
}

const bdAddr& bdIPDiscoveryPacketReply::getAddr()
{
    return m_addr;
}

bdUInt16 bdIPDiscoveryPacketReply::getProtocolVersion()
{
    return m_protocolVersion;
}
