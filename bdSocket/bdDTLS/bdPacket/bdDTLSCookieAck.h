// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdDTLSCookieAck : public bdDTLSHeader
{
protected:
    bdUByte8 m_ECCKey[100];
    bdSecurityID m_secID;
public:
    void operator delete(void* p);
    bdDTLSCookieAck();
    bdDTLSCookieAck(bdUInt16 vtag, const bdUByte8* key, const bdSecurityID secID);
    virtual ~bdDTLSCookieAck();
    virtual bdBool serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset);
    virtual bdBool deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset);
    bdUByte8* getECCKey();
    void getSecID(bdSecurityID& secID);
};
