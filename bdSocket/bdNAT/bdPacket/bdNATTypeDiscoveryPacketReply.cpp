// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocket.h"

bdNATTypeDiscoveryPacketReply::bdNATTypeDiscoveryPacketReply()
    : m_type(0), m_protocolVersion(0), m_mappedAddr(), m_secAddr()
{
}

bdNATTypeDiscoveryPacketReply::bdNATTypeDiscoveryPacketReply(const bdAddr& mappedAddr, const bdAddr& secAddr, const bdUInt16 protocolVersion)
    : m_type(21), m_protocolVersion(protocolVersion), m_mappedAddr(mappedAddr), m_secAddr(secAddr)
{
}

bdBool bdNATTypeDiscoveryPacketReply::serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset)
{
    bdBool status = true;

    newOffset = offset;
    AppendBasicType(status, bdUByte8, data, size, newOffset, newOffset, m_type);
    if (m_protocolVersion == 1)
    {
        SkipBytes(status, data, size, newOffset, newOffset, 1u);
    }
    AppendBasicType(status, bdUInt16, data, size, newOffset, newOffset, m_protocolVersion);
    Serialize(status, m_mappedAddr, data, size, newOffset, newOffset);
    if (m_protocolVersion == 1)
    {
        SkipBytes(status, data, size, newOffset, newOffset, 2u);
    }
    Serialize(status, m_secAddr, data, size, newOffset, newOffset);
    if (m_protocolVersion == 1)
    {
        SkipBytes(status, data, size, newOffset, newOffset, 2u);
    }
    if (!status)
    {
        newOffset = offset;
        return false;
    }
    return status;
}

bdBool bdNATTypeDiscoveryPacketReply::deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset)
{
    bdBool status = true;

    newOffset = offset;
    RemoveBasicType(status, bdUByte8, data, size, newOffset, newOffset, m_type);
    if (m_type < 0x14 || m_type > 0x1D)
    {
        newOffset = offset;
        return false;
    }
    RemoveBasicType(status, bdUInt16, data, size, newOffset, newOffset, m_protocolVersion);
    if (m_protocolVersion != 2)
    {
        RewindBytes(status, data, size, newOffset, newOffset, 2u);
        SkipBytes(status, data, size, newOffset, newOffset, 2u);
        RemoveBasicType(status, bdUInt16, data, size, newOffset, newOffset, m_protocolVersion);
        if (m_protocolVersion != 1)
        {
            newOffset = offset;
            return false;
        }
    }
    Deserialize(status, m_mappedAddr, data, size, newOffset, newOffset);
    if (m_protocolVersion == 1)
    {
        SkipBytes(status, data, size, newOffset, newOffset, 2u);
    }
    Deserialize(status, m_secAddr, data, size, newOffset, newOffset);
    if (!status)
    {
        newOffset = offset;
    }
    return status;
}

bdUByte8 bdNATTypeDiscoveryPacketReply::getType() const
{
    return m_type;
}

bdUInt16 bdNATTypeDiscoveryPacketReply::getVersion() const
{
    return m_protocolVersion;
}

const bdAddr& bdNATTypeDiscoveryPacketReply::getMappedAddr() const
{
    return m_mappedAddr;
}

const bdAddr& bdNATTypeDiscoveryPacketReply::getSecAddr() const
{
    return m_secAddr;
}
