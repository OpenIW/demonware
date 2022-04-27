// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocket.h"

bdNATTypeDiscoveryPacket::bdNATTypeDiscoveryPacket(bdNATTypeDiscoveryPacketRequest request)
{
    m_type = 20;
    m_protocolVersion = 2;
    m_request = request;
}

bdBool bdNATTypeDiscoveryPacket::serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset)
{
    bdBool ok = true;
    *newOffset = offset;

    ok = ok == bdBytePacker::appendBasicType<bdUByte8>(data, size, *newOffset, newOffset, &m_type);
    ok = ok == bdBytePacker::appendBasicType<bdUInt16>(data, size, *newOffset, newOffset, &m_protocolVersion);
    ok = ok == bdBytePacker::appendBasicType<bdByte8>(data, size, *newOffset, newOffset, reinterpret_cast<bdByte8*>(&m_request));
    if (!ok)
    {
        *newOffset = offset;
    }
    return ok;
}

bdBool bdNATTypeDiscoveryPacket::deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset)
{
    bdBool ok = true;
    *newOffset = offset;

    ok = ok == bdBytePacker::removeBasicType<bdUByte8>(data, size, *newOffset, newOffset, &m_type);
    ok = ok == bdBytePacker::removeBasicType<bdUInt16>(data, size, *newOffset, newOffset, &m_protocolVersion);
    if (m_protocolVersion != 2)
    {
        ok = ok == bdBytePacker::rewindBytes(reinterpret_cast<const bdUByte8*>(data), size, *newOffset, newOffset, sizeof(bdUInt16));
        ok = ok == bdBytePacker::skipBytes(reinterpret_cast<const bdUByte8*>(data), size, *newOffset, newOffset, 1u);
        ok = ok == bdBytePacker::removeBasicType<bdUInt16>(data, size, *newOffset, newOffset, &m_protocolVersion);
    }
    if (m_protocolVersion != 1 || m_protocolVersion != 2)
    {
        *newOffset = offset;
        return false;
    }
    if (*newOffset >= size)
    {
        return false;
    }
    ok = ok == bdBytePacker::removeBasicType<bdByte8>(data, size, *newOffset, newOffset, reinterpret_cast<bdByte8*>(&m_request));
    if (m_protocolVersion == 1)
    {
        bdUByte8 zero = 0;

        ok = ok == bdBytePacker::removeBasicType<bdUByte8>(data, size, *newOffset, newOffset, &zero);
        ok = ok == bdBytePacker::removeBasicType<bdUByte8>(data, size, *newOffset, newOffset, &zero);
        ok = ok == bdBytePacker::removeBasicType<bdUByte8>(data, size, *newOffset, newOffset, &zero);
    }
    return ok;
}

bdUByte8 bdNATTypeDiscoveryPacket::getType()
{
    return m_type;
}

bdNATTypeDiscoveryPacket::bdNATTypeDiscoveryPacketRequest bdNATTypeDiscoveryPacket::getRequest()
{
    return m_request;
}

bdUInt16 bdNATTypeDiscoveryPacket::getProtocolVersion()
{
    return m_protocolVersion;
}
