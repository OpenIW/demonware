// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdIPDiscoveryPacket
{
protected:
    bdUByte8 m_type;
    bdUInt16 m_protocolVersion;
public:
    bdIPDiscoveryPacket();
    bdIPDiscoveryPacket(bdUInt16 version);
    bdBool serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset);
    bdBool deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset);
    bdUByte8 getType();
    bdUInt16 getProtocolVersion();
};