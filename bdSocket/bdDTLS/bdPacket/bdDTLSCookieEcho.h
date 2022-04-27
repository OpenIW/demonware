// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdDTLSCookieEcho : public bdDTLSHeader
{
protected:
    bdDTLSInitAck m_cookie;
    bdUByte8 m_ca[25];
    bdUByte8 m_secID[8];
    bdUByte8 m_ECCKey[100];
public:
    void operator delete(void* p);
    bdDTLSCookieEcho();
    bdDTLSCookieEcho(bdUInt16 vtag, bdDTLSInitAck* cookie, bdCommonAddrRef localCommonAddr, bdECCKey* ECCKey);
    virtual ~bdDTLSCookieEcho();
    virtual bdBool serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset);
    virtual bdBool deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset);
    const bdDTLSInitAck* getCookie() const;
    const bdUByte8* getECCKey() const;
    const bdUByte8* getCa() const;
    const bdUByte8* getSecID() const;
};