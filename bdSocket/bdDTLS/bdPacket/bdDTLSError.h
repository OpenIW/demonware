// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdDTLSError : public bdDTLSHeader
{
public:
    enum bdDTLSErrorType : bdInt
    {
        BD_DTLS_ERROR_BAD_SECID = 0x0,
        BD_DTLS_INVALID_STATE = 0x1,
    };
protected:
    bdDTLSErrorType m_etype;
    bdSecurityID m_secID;
public:
    void operator delete(void* p);
    bdDTLSError();
    bdDTLSError(bdUInt16 vtag, bdDTLSErrorType etype, const bdSecurityID& secID);
    virtual ~bdDTLSError();
    virtual bdBool serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset);
    virtual bdBool deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset);
    bdInt getEtype();
    void getSecID(bdSecurityID& secID);
};
