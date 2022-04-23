// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#define BD_NAT_TRAV_HMAC_SIZE 10

class bdNATTraversalPacket
{
public:
    static bdUInt serializedSize;
protected:
    bdUByte8 m_type;
    bdUInt16 m_protocolVersion;
    bdUByte8 m_hmac[BD_NAT_TRAV_HMAC_SIZE];
    bdUInt m_ident;
    bdAddr m_addrSrc;
    bdAddr m_addrDest;
public:
    bdNATTraversalPacket();
    bdNATTraversalPacket(const bdUByte8* m_type, const bdUInt ident, const bdAddr* addrSrc, const bdAddr* addrDest);
    ~bdNATTraversalPacket();
    bdBool deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset);
    bdBool serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset);

    bdUByte8 getType();
    bdUInt16 getProtocolVersion();
    bdUByte8* getHMAC();
    bdUInt getIdentifier();
    bdUInt getSize();
    const bdAddr* getAddrSrc();
    const bdAddr* getAddrDest();

    void setType(bdUByte8 type);
    void setHMAC(const bdUByte8* hmac, const bdUInt size);
};