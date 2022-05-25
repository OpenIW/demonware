// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdNATTypeDiscoveryPacket
{
public:
    enum bdNATTypeDiscoveryPacketRequest : bdInt
    {
        BD_NTDP_SAME_ADDR = 0x0,
        BD_NTDP_DIFF_PORT = 0x1,
        BD_NTDP_DIFF_IP = 0x2,
        BD_NTDP_DIFF_PORT_IP = 0x3,
        BD_NTDP_INVALID_TYPE = 0x4,
    };
protected:
    bdUByte8 m_type;
    bdUInt16 m_protocolVersion;
    bdNATTypeDiscoveryPacketRequest m_request;
public:
    bdNATTypeDiscoveryPacket(bdNATTypeDiscoveryPacketRequest request);
    bdBool serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset);
    bdBool deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset);
    bdUByte8 getType();
    bdNATTypeDiscoveryPacketRequest getRequest();
    bdUInt16 getProtocolVersion();
};
