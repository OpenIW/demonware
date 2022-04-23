// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdNATTypeDiscoveryPacketReply
{
protected:
    bdUByte8 m_type;
    bdUInt16 m_protocolVersion;
    bdAddr m_mappedAddr;
    bdAddr m_secAddr;
public:
    bdNATTypeDiscoveryPacketReply();
    bdNATTypeDiscoveryPacketReply(const bdAddr* mappedAddr, const bdAddr* secAddr, const bdUInt16 protocolVersion);
    bdBool serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset);
    bdBool deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset);
    bdUByte8 getType();
    bdUInt16 getVersion();
    bdAddr* getMappedAddr();
    bdAddr* getSecAddr();
};