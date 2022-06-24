// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdConnection/bdConnection.h"

void bdPacket::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdPacket::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdPacket::bdPacket()
    : m_chunks(), m_me(), m_tag(0), m_bufferSize(0), m_currentSize(0)
{
}

bdPacket::bdPacket(const bdUInt32 tag, const bdUInt bufferSize)
    : m_chunks(), m_me(), m_tag(tag), m_bufferSize(bufferSize - 6), m_currentSize(6)
{
}

bdUInt bdPacket::serialize(bdUByte8* outBuffer, const bdUInt outSize)
{
    bdUInt serializedNumBytes;
    bdUInt bytesWritten;
    bdUInt outOffset = 2;
    bdBool ok = bdBytePacker::appendBasicType<bdUInt>(outBuffer, outSize, outOffset, outOffset, m_tag);
    bdUInt bytesRemaining = outSize - outOffset;
    bdUInt encDataSize = outOffset - 2;

    for (Position chunkPos = m_chunks.getHeadPosition(); chunkPos; m_chunks.forward(chunkPos))
    {
        bdChunkRef chunk(*reinterpret_cast<bdChunkRef*>(chunkPos));
        serializedNumBytes = chunk->getSerializedSize();
        if (bytesRemaining < serializedNumBytes)
        {
            bdLogWarn("bdConnection/packet", "Buffer not big enough for all chunks.");
            break;
        }
        bytesRemaining -= serializedNumBytes;
        bytesWritten = chunk->serialize(&outBuffer[outOffset], outSize - outOffset);
        encDataSize += bytesWritten;
        outOffset += bytesWritten;
    }
    for (Position chunkPos = m_chunks.getHeadPosition(); chunkPos; m_chunks.forward(chunkPos))
    {
        bdChunkRef chunk(*reinterpret_cast<bdChunkRef*>(chunkPos));
        if (chunk->getType() == 2)
        {
            bdDataChunkRef dataChunk(reference_cast<bdDataChunk,bdChunk>(bdChunkRef(chunk)));
            bytesWritten = dataChunk->serializeUnencrypted(&outBuffer[outOffset], outSize - outOffset);
            outOffset += bytesWritten;
        }
    }
    bdUInt totalBytesWritten = 0;
    if (ok)
    {
        bdUInt16 encDataSize16 = static_cast<bdUInt16>(encDataSize);
        if (encDataSize16 != encDataSize)
        {
            bdLogError("bdConnection/packet", "Encrypted data section size too big to fit in bdUInt16.");
        }
        else
        {
            bdUInt newOffset = 0;
            if (bdBytePacker::appendBasicType<bdUInt16>(outBuffer, outSize, 0, newOffset, encDataSize16))
            {
                totalBytesWritten = outOffset;
            }
        }
    }
    bdAssert(m_currentSize == totalBytesWritten, "Packet size miscalculation.");
    return totalBytesWritten;
}

bdBool bdPacket::deserialize(const bdUByte8* inData, const bdUInt inSize)
{
    bdUInt unencOffset;
    bdUInt encOffset = 0;
    bdUInt16 encSize = 0;
    bdBool ok = false;
    bdChunkTypes type;

    if (inData && inSize > 6)
    {
        ok = bdBytePacker::removeBasicType<bdUInt16>(inData, inSize, encOffset, encOffset, encSize);
    }
    if (encSize > inSize - encOffset)
    {
        bdLogError("bdConnection/packet", "Received a malformed packet!  Packet claims to have size [%u] bytes, but we only received [%u] bytes of data!", encSize, inSize);
        return false;
    }
    const bdUByte8* const encData = &inData[encOffset];
    const bdUByte8* const unencData = &encData[encSize];
    bdUInt unencSize = inSize - (encOffset + encSize);
    unencOffset = 0;
    encOffset = 0;

    ok = ok == bdBytePacker::removeBasicType<bdUInt32>(encData, encSize, encOffset, encOffset, m_tag);
    for (bdUInt iterations = 0; encOffset < encSize && ok && iterations < 100; ++iterations)
    {
        type = bdChunk::getType(&encData[encOffset], encSize - encOffset);
        bdChunkRef chunk;
        switch (type)
        {
        case BD_CT_INVALID:
            break;
        case BD_CT_PADDING:
            ++encOffset;
            break;
        case BD_CT_DATA:
            chunk = new bdDataChunk();
            break;
        case BD_CT_INIT:
            chunk = new bdInitChunk();
            break;
        case BD_CT_INIT_ACK:
            chunk = new bdInitAckChunk();
            break;
        case BD_CT_SACK:
            chunk = new bdSAckChunk();
            break;
        case BD_CT_HEARTBEAT:
            chunk = new bdHeartbeatChunk();
            break;
        case BD_CT_HEARTBEAT_ACK:
            chunk = new bdHeartbeatAckChunk();
            break;
        case BD_CT_ABORT:
            bdLogWarn("bdConnection/packet", "Not implemented! (BD_CT_ABORT)");
            break;
        case BD_CT_SHUTDOWN:
            chunk = new bdShutdownChunk();
            break;
        case BD_CT_SHUTDOWN_ACK:
            chunk = new bdShutdownAckChunk();
            break;
        case BD_CT_SHUTDOWN_COMPLETE:
            chunk = new bdShutdownCompleteChunk();
            break;
        case BD_CT_ERROR:
            bdLogWarn("bdConnection/packet", "Not implemented! (BD_CT_ERROR)");
            break;
        case BD_CT_COOKIE_ECHO:
            chunk = new bdCookieEchoChunk();
            break;
        case BD_CT_COOKIE_ACK:
            chunk = new bdCookieAckChunk();
            break;
        default:
            ok = false;
            bdLogWarn("bdConnection/packet", "unknown chunk type.");
            break;
        }
        if (!*chunk || !ok)
        {
            continue;
        }
        if (type == BD_CT_DATA)
        {
            bdDataChunkRef dataChunk(reference_cast<bdDataChunk, bdChunk>(bdChunkRef(chunk)));
            ok = ok == dataChunk->deserialize(encData, encSize, encOffset, unencData, unencSize, unencOffset);
        }
        else
        {
            ok = ok == chunk->deserialize(encData, encSize, encOffset);
        }

        if (!ok)
        {
            bdLogWarn("bdConnection/packet", "Chunk deserialization failed.");
            continue;
        }
        m_chunks.addTail(chunk);
    }
    return ok;
}

bdBool bdPacket::addChunk(bdChunkRef chunk)
{
    bdAssert(chunk.notNull(), "Chunk should never be null.");
    bdUInt newPacketSize = chunk->getSerializedSize() + m_currentSize;
    if (newPacketSize <= m_bufferSize)
    {
        m_chunks.addTail(chunk);
        m_currentSize = newPacketSize;
        return true;
    }
    return false;
}

bdBool bdPacket::getNextChunk(bdChunkRef& chunk)
{
    if (!m_chunks.isEmpty())
    {
        chunk = m_chunks.getHead();
        m_chunks.removeHead();
        return true;
    }
    return false;
}

const bdBool bdPacket::isEmpty() const
{
    return m_chunks.isEmpty();
}

const bdUInt32 bdPacket::getVerificationTag() const
{
    return m_tag;
}
