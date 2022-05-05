// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdConnection/bdConnection.h"

void bdDataChunk::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdDataChunk::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdDataChunk::bdDataChunk()
    : bdChunk(BD_CT_DATA), m_message(), m_flags(0), m_seqNum(0)
{
}

bdDataChunk::bdDataChunk(bdMessageRef data, const bdDataChunk::bdDataFlags flags)
    : bdChunk(BD_CT_DATA), m_message(data), m_flags(flags), m_seqNum(0)
{
    if (m_message.isNull())
    {
        return;
    }
    if (m_message->hasPayload() && m_message->getPayload()->getNumBitsWritten() != 0)
    {
        m_flags |= BD_DC_ENC_DATA;
    }
    if (m_message->hasUnencryptedPayload() && m_message->getUnencryptedPayload()->getMaxReadSize() != 0)
    {
        m_flags |= BD_DC_UNENC_DATA;
    }
}

bdUInt bdDataChunk::serialize(bdUByte8* data, const bdUInt32 size)
{
    bdUByte8* payloadData;
    bdUInt offset = bdChunk::serialize(data, size);
    bdBool ok = true;

    if (m_message.isNull())
    {
        return offset;
    }
    ok = bdBytePacker::appendBasicType<bdUByte8>(data, size, offset, &offset, reinterpret_cast<bdUByte8*>(&m_flags));
    bdUByte8 type = m_message->getType();
    ok = ok == bdBytePacker::appendBasicType<bdUByte8>(data, size, offset, &offset, &type);
    ok = ok == bdBytePacker::appendBasicType<bdSeqNumber>(data, size, offset, &offset, &m_seqNum);
    bdUInt16 payloadSize = 0;
    if ((m_flags & BD_DC_ENC_DATA) != 0)
    {
        bdAssert(m_message->hasPayload(), "BD_DC_ENC_DATA flag set but no payload.");
        bdBitBufferRef encPayload(m_message->getPayload());
        if (encPayload.notNull())
        {
            payloadSize = encPayload->getNumBitsWritten();
            if (payloadSize)
            {
                payloadSize = encPayload->getDataSize();
            }
        }
        ok = ok == bdBytePacker::appendEncodedUInt16(data, size, offset, &offset, payloadSize);
    }
    if ((m_flags & BD_DC_UNENC_DATA) != 0)
    {
        bdAssert(m_message->hasUnencryptedPayload(), "BD_DC_UNENC_DATA flag set but no unencrypted payload.");
        bdUInt16 unencPayloadSize = 0;
        bdByteBufferRef unencPayload(m_message->getUnencryptedPayload());
        if (unencPayload.notNull())
        {
            unencPayloadSize = unencPayload->getDataSize();
        }
        ok = ok == bdBytePacker::appendEncodedUInt16(data, size, offset, &offset, unencPayloadSize);
    }
    if (payloadSize)
    {
        payloadData = m_message->getPayload()->getData();
        ok = ok == bdBytePacker::appendBuffer(data, size, offset, &offset, payloadData, payloadSize);
    }
    return ok ? offset : 0;
}

bdUInt bdDataChunk::serializeUnencrypted(bdUByte8* data, const bdUInt32 size)
{
    const bdUByte8* unEncData;
    bdUInt unEncDataSize;
    bdUInt offset;
    bdUInt bytesWritten = 0;

    if ((m_flags & BD_DC_UNENC_DATA) == 0)
    {
        return bytesWritten;
    }
    bdAssert(m_message->hasUnencryptedPayload(), "BD_DC_UNENC_DATA flag set but no unencrypted payload.");
    bdByteBufferRef unEncPayload(m_message->getUnencryptedPayload());
    if (unEncPayload.notNull())
    {
        unEncData = unEncPayload->getData();
        unEncDataSize = unEncPayload->getDataSize();
        if (bdBytePacker::appendBuffer(data, size, 0, &offset, unEncData, unEncDataSize))
        {
            bytesWritten = offset;
        }
    }
    return bytesWritten;
}

bdBool bdDataChunk::deserialize(const bdUByte8* const data, const bdUInt size, bdUInt* offset)
{
    bdUInt unencOffset = 0;
    return deserialize(data, size, offset, NULL, NULL, &unencOffset);
}

bdBool bdDataChunk::deserialize(const bdUByte8* const data, const bdUInt size, bdUInt* offset, const bdUByte8* const unencData, const bdUInt unencSize, bdUInt* unencOffset)
{
    bdUInt16 unencPayloadSize;
    bdUInt16 payloadSize;

    bdUInt bytesRead = *offset;
    bdUInt unencBytesRead = *unencOffset;

    bdBool ok = bdChunk::deserialize(data, size, &bytesRead);
    ok = ok == bdBytePacker::removeBasicType<bdUByte8>(data, size, bytesRead, &bytesRead, &m_flags);
    bdUByte8 type8 = 0;
    ok = ok == bdBytePacker::removeBasicType<bdUByte8>(data, size, bytesRead, &bytesRead, &type8);
    ok = ok == bdBytePacker::removeBasicType<bdSeqNumber>(data, size, bytesRead, &bytesRead, &m_seqNum);
    if ((m_flags & BD_DC_ENC_DATA) != 0)
    {
        ok = ok == bdBytePacker::removeEncodedUInt16(data, size, bytesRead, &bytesRead, &payloadSize);
        if (payloadSize > size - bytesRead)
        {
            bdLogError("bdDataChunk",
                "Received a malformed packet!  Data chunk claims to have size [%u] bytes, but there are only [%u] bytes of data remaining in the buffer!",
                payloadSize, size - bytesRead);
            ok = false;
        }
    }
    if ((m_flags & BD_DC_UNENC_DATA) != 0)
    {
        ok = ok == bdBytePacker::removeEncodedUInt16(data, size, bytesRead, &bytesRead, &unencPayloadSize);
        if (unencPayloadSize > size - bytesRead)
        {
            bdLogError("bdDataChunk",
                "Received a malformed packet!  Data chunk claims to have unencrypted payload of [%u] bytes, but there are only [%u] bytes of data remaining in the buffer!",
                unencPayloadSize, size - bytesRead);
            ok = false;
        }
    }
    if (ok)
    {
        m_message = new bdMessage(type8, &data[bytesRead], payloadSize, true, &unencData[unencBytesRead], unencPayloadSize);
        ok = m_message.notNull();
        bytesRead += payloadSize;
        unencBytesRead += unencPayloadSize;
        *offset = bytesRead;
        *unencOffset = unencBytesRead;
    }
    return ok;
}

bdUInt bdDataChunk::getSerializedSize()
{
    return serialize(NULL, NULL);
}

void bdDataChunk::setSequenceNumber(bdSeqNumber seqNum)
{
    m_seqNum = seqNum;
}

const bdSeqNumber bdDataChunk::getSequenceNumber() const
{
    return m_seqNum;
}

bdMessageRef bdDataChunk::getMessage() const
{
    return bdMessageRef(&m_message);
}

const bdUByte8 bdDataChunk::getFlags() const
{
    return m_flags;
}
