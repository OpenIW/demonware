// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdDTLSInitAck : public bdDTLSHeader
{
protected:
    bdUInt m_timestamp;
    bdUInt m_signature;
    bdUInt16 m_initTag;
    bdUInt16 m_localTag;
    bdUInt16 m_peerTag;
    bdUInt16 m_localTieTag;
    bdUInt16 m_peerTieTag;
    bdAddr m_peerAddr;
    bdSecurityID m_secID;
public:
    void operator delete(void* p);
    bdDTLSInitAck();
    bdDTLSInitAck(bdUInt16 vtag, bdUInt16 initTag, bdUInt16 localTag, bdUInt16 peerTag, bdUInt16 localTieTag, bdUInt16 peerTieTag, bdUInt32 timestamp, const bdAddr* peerAddr, bdSecurityID secID);
    bdDTLSInitAck(const bdDTLSInitAck* other);
    bdDTLSInitAck* operator=(bdDTLSInitAck* other);
    virtual ~bdDTLSInitAck();
    virtual bdBool serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset);
    virtual bdBool deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset);
    void sign(bdHMacSHA1* hmac);
    bdBool verify(bdHMacSHA1* hmac);
    const bdUInt16 getLocalTag() const;
    const bdUInt16 getPeerTag() const;
    const bdUInt16 getLocalTieTag() const;
    const bdUInt16 getPeerTieTag() const;
    const bdUInt16 getInitTag() const;
    const bdAddr* getPeerAddr() const;
    const bdUInt getTimestamp() const;
    void getSecID(bdSecurityID* secID) const;
};