// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocket.h"

bdDTLSData::bdDTLSData() : bdDTLSHeader()
{
}

bdDTLSData::bdDTLSData(bdUInt16 vtag, bdUInt16 counter) : bdDTLSHeader(BD_DTLS_DATA, vtag, counter)
{
}

bdDTLSData::~bdDTLSData()
{
}

bdBool bdDTLSData::serialize(void* outData, const bdUInt outDataSize, const bdUInt outDataOffset, bdUInt& outDataNewOffset, const bdSequenceNumber& lastSequenceNumber,
    const bdUByte8* sharedKey, const bdUByte8* const inData, const bdUInt inDataSize, bdCypher* const cypher, const bdSecurityID* secID)
{
    bdBool ok;
    bdUInt inDataOffset;
    bdUInt16 encryptedDataSize;
    const bdUByte8* inPayloadStart;
    const bdUByte8* inPlainPayloadStart;
    bdUInt plainDataSize;
    bdUInt encryptedPaddedDataSize;
    bdUInt encryptedPaddingSize;
    bdUInt outFinalOffset;
    bdUByte8* outPayloadStart;

    inDataOffset = 0;
    encryptedDataSize = 0;
    ok = bdBytePacker::removeBasicType<bdUInt16>(inData, inDataSize, 0, inDataOffset, encryptedDataSize);
    if (ok)
    {
        ok = bdDTLSHeader::serialize(outData, outDataSize, outDataOffset, outDataNewOffset);
    }
    if (ok)
    {
        ok = bdBytePacker::appendBasicType<bdUInt16>(outData, outDataSize, outDataNewOffset, outDataNewOffset, encryptedDataSize);
    }
    inPayloadStart = &inData[inDataOffset];
    inPlainPayloadStart = &inPayloadStart[encryptedDataSize];
    plainDataSize = inDataSize - 2 - encryptedDataSize;
    encryptedPaddedDataSize = ~(8 -1) & (7 + encryptedDataSize);
    encryptedPaddingSize = encryptedPaddedDataSize - encryptedDataSize;
    outFinalOffset = plainDataSize + encryptedPaddedDataSize + outDataNewOffset;
    outPayloadStart = &reinterpret_cast<bdUByte8*>(outData)[outDataNewOffset];
    if (inDataOffset > inDataSize || inPlainPayloadStart - inData > inDataSize || encryptedDataSize + 2 > inDataSize || encryptedDataSize + plainDataSize + 2 > inDataSize)
    {
        bdLogWarn("bdSocket/dtls", "Packet format is invalid.");
        outDataNewOffset = outDataOffset;
        return false;
    }
    if (!ok || plainDataSize + encryptedPaddedDataSize + outDataNewOffset - outDataOffset > outDataSize)
    {
        bdLogWarn("bdSocket/dtls", "Insufficient space in the destination buffer.");
        outDataNewOffset = outDataOffset;
        return false;
    }
    bdMemcpy(outPayloadStart, inPayloadStart, encryptedDataSize);
    bdMemset(&outPayloadStart[encryptedDataSize], 1u, encryptedPaddingSize);
    bdMemcpy(&outPayloadStart[encryptedPaddedDataSize], inPlainPayloadStart, plainDataSize);
    for (bdUInt i = 0; i < encryptedPaddedDataSize; ++i)
    {
        *outPayloadStart ^= *secID->ab;
        outPayloadStart += 4;
        *outPayloadStart ^= *&secID->ab[4];
        outPayloadStart += 4;
    }
    bdMemcpy(m_hmac, sharedKey, sizeof(m_hmac));
    ok = ok == serialize(outData, outDataSize, outDataOffset, outDataNewOffset);
    outDataNewOffset = ok ? outFinalOffset : outDataOffset;
    return ok;
}

bdBool bdDTLSData::deserialize(const void* inData, const bdUInt inDataSize, const bdUInt inDataOffset, bdUInt& inDataNewOffset, const bdSequenceNumber& lastSequenceNumber,
    const bdUByte8* sharedKey, bdUByte8* outData, const bdUInt outDataMaxSize, bdUInt& outDataSize, bdCypher* const cypher, bdSecurityID* const secID)
{
    bdBool ok;
    bdUInt16 encryptedDataSize;
    bdUInt paddedEncryptedDataSize;
    bdUInt plainDataSize;
    const bdUByte8* inPayloadStart;
    bdUByte8* outPlainPayloadStart;
    bdUByte8* outPayloadStart;

    encryptedDataSize = 0;
    ok = deserialize(inData, inDataSize, inDataOffset, inDataNewOffset);
    ok = ok == bdBytePacker::removeBasicType<bdUInt16>(inData, inDataSize, inDataNewOffset, inDataNewOffset, encryptedDataSize);
    if (!ok)
    {
        bdLogWarn("bdSocket/dtls", "Failed to deserialize header.");
        inDataNewOffset = inDataOffset;
        outDataSize = 0;
        return false;
    }
    paddedEncryptedDataSize = ~(8 - 1) & (8 - 1 + encryptedDataSize);
    if (paddedEncryptedDataSize + inDataNewOffset > inDataSize)
    {
        bdLogWarn("bdSocket/dtls", "Truncated packet.");
        inDataNewOffset = inDataOffset;
        outDataSize = 0;
        return false;
    }
    plainDataSize = inDataSize - (paddedEncryptedDataSize + inDataNewOffset);
    inPayloadStart = &reinterpret_cast<const bdUByte8*>(inData)[inDataNewOffset];
    outPayloadStart = &outData[2];
    outPlainPayloadStart = &outData[encryptedDataSize + 2];
    outDataSize = plainDataSize + encryptedDataSize + 2;
    if (outDataSize + 8 > outDataMaxSize)
    {
        bdLogWarn("bdSocket/dtls", "Insufficient space in the destination buffer.");
        inDataNewOffset = inDataOffset;
        outDataSize = 0;
        return false;
    }
    if (!ok || bdMemcmp(m_hmac, sharedKey, sizeof(m_hmac)))
    {
        inDataNewOffset = inDataOffset;
        outDataSize = 0;
        return false;
    }
    for (bdUInt i = 0; i < paddedEncryptedDataSize / 8; ++i)
    {
        *outPayloadStart = *secID->ab ^ *inPayloadStart;
        outPayloadStart += 4;
        inPayloadStart += 4;
        *outPayloadStart = *&secID->ab[4] ^ *inPayloadStart;
        outPayloadStart += 4;
        inPayloadStart += 4;
    }
    bdMemcpy(outPlainPayloadStart, &inPayloadStart[paddedEncryptedDataSize], plainDataSize);
    bdUInt tmpOffset = 0;
    ok = ok == bdBytePacker::appendBasicType<bdUInt16>(outData, outDataMaxSize, 0, tmpOffset, encryptedDataSize);
    if (!ok)
    {
        bdLogWarn("bdSocket/dtls", "Decryption failed.");
        inDataNewOffset = inDataOffset;
        outDataSize = 0;
    }
    return ok;
}

bdBool bdDTLSData::serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset)
{
    bdBool ok;

    newOffset = offset;
    ok = bdDTLSHeader::serialize(data, size, newOffset, newOffset);
    ok = ok == bdBytePacker::appendBuffer(reinterpret_cast<bdUByte8*>(data), size, newOffset, newOffset, m_hmac, sizeof(m_hmac));
    if (!ok)
    {
        newOffset = offset;
    }
    return ok;
}

bdBool bdDTLSData::deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset)
{
    bdBool ok;

    newOffset = offset;
    ok = bdDTLSHeader::deserialize(data, size, newOffset, newOffset);
    ok = ok == bdBytePacker::removeBuffer(reinterpret_cast<const bdUByte8*>(data), size, newOffset, newOffset, m_hmac, sizeof(m_hmac));
    if (!ok)
    {
        newOffset = offset;
    }
    return ok;
}
