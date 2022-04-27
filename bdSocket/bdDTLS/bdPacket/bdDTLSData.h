// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdDTLSData : public bdDTLSHeader
{
protected:
    bdUByte8 m_hmac[8];
public:
    bdDTLSData();
    virtual bdBool serialize(void* outData, const bdUInt outDataSize, const bdUInt outDataOffset, bdUInt* outDataNewOffset, const bdSequenceNumber* lastSequenceNumber, const bdUByte8* sharedKey,
        const bdUByte8* const inData, const bdUInt inDataSize, bdCypher* const cypher, bdSecurityID* const secID);
    virtual bdBool deserialize(const void* inData, const bdUInt inDataSize, const bdUInt inDataOffset, bdUInt* inDataNewOffset, const bdSequenceNumber* lastSequenceNumber, const bdUByte8* sharedKey,
        bdUByte8* outData, const bdUInt outDataMaxSize, bdUInt* outDataSize, bdCypher* const cypher, bdSecurityID* const secID);
    virtual bdBool serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset);
    virtual bdBool deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset);
};