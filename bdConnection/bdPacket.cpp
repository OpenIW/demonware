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
    return bdUInt();
}

bdBool bdPacket::deserialize(const bdUByte8* inData, const bdUInt inSize)
{
    return bdBool();
}

bdBool bdPacket::addChunk(bdChunkRef chunk)
{
    bdAssert(chunk.notNull(), "Chunk should never be null.");
    bdUInt newPacketSize = chunk->getSerializedSize() + m_currentSize;
    if (newPacketSize <= m_bufferSize)
    {
        m_chunks.addTail(&chunk);
        m_currentSize = newPacketSize;
        return true;
    }
    return false;
}

bdBool bdPacket::getNextChunk(bdChunkRef* chunk)
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
