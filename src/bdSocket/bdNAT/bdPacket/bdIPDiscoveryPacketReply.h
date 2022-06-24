// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdIPDiscoveryPacketReply
{
protected:
    bdUByte8 m_type;
    bdUInt16 m_protocolVersion;
    bdAddr m_addr;
public:
    bdIPDiscoveryPacketReply();
    bdIPDiscoveryPacketReply(const bdAddr& addr, bdUInt16 protocolVersion);
    ~bdIPDiscoveryPacketReply();
    bdBool serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset);
    bdBool deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset);
    bdUByte8 getType();
    const bdAddr& getAddr();
    bdUInt16 getProtocolVersion();
};
