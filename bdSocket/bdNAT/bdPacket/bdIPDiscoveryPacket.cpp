// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocketRouter.h"

bdIPDiscoveryPacket::bdIPDiscoveryPacket()
{
    m_type = 30;
    m_protocolVersion = 2;
}

bdIPDiscoveryPacket::bdIPDiscoveryPacket(bdUInt16 version)
{
    m_type = 30;
    m_protocolVersion = version;
}

bdBool bdIPDiscoveryPacket::serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset)
{
    *newOffset = offset;
    bdBool appended = false;
    if (bdBytePacker::appendBasicType<bdUByte8>(data, size, *newOffset, newOffset, &m_type))
    {
        appended = bdBytePacker::appendBasicType<bdUInt16>(data, size, *newOffset, newOffset, &m_protocolVersion);
    }
    if (!appended)
    {
        *newOffset = offset;
    }
    return appended;
}

bdBool bdIPDiscoveryPacket::deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset)
{
    bdBool status = false;

    *newOffset = offset;
    if (bdBytePacker::removeBasicType<bdUByte8>(data, size, *newOffset, newOffset, &m_type))
    {
        status = bdBytePacker::removeBasicType<bdUInt16>(data, size, *newOffset, newOffset, &m_protocolVersion);
    }
    if (m_protocolVersion != 1)
    {
        status = status && bdBytePacker::rewindBytes(reinterpret_cast<const bdUByte8*>(data), size, *newOffset, newOffset, 2u);
        status = status && bdBytePacker::skipBytes(reinterpret_cast<const bdUByte8*>(data), size, *newOffset, newOffset, 1u);
    }
    // Check again
    if (m_protocolVersion != 1)
    {
        *newOffset = offset;
        return false;
    }
    if (!status)
    {
        *newOffset = offset;
    }
    return status;
}

bdUByte8 bdIPDiscoveryPacket::getType()
{
    return m_type;
}

bdUInt16 bdIPDiscoveryPacket::getProtocolVersion()
{
    return m_protocolVersion;
}
