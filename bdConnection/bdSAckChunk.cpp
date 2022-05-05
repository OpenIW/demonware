// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdConnection/bdConnection.h"

void* bdSAckChunk::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

void bdSAckChunk::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

bdSAckChunk::bdSAckChunk()
    : bdChunk(BD_CT_SACK), m_flags(BD_SC_NO_FLAGS), m_cumulativeAck(0), m_gaps(), m_windowCredit(0)
{
}

bdSAckChunk::bdSAckChunk(const bdWord windowCredit, bdSAckFlags flags)
    : bdChunk(BD_CT_SACK), m_flags(flags), m_cumulativeAck(0), m_gaps(), m_windowCredit(windowCredit)
{
}

bdUInt bdSAckChunk::serialize(bdUByte8* data, const bdUInt32 size)
{
    const bdSAckChunk::bdGapAckBlock* gap;
    Position pos;
    bdUInt16 end;
    bdUInt16 start;
    bdUInt16 numGaps;
    bdBool ok = false;
    
    bdUInt offset = bdChunk::serialize(data, size);

    ok = bdBytePacker::appendBasicType<bdUByte8>(data, size, offset, &offset, reinterpret_cast<bdUByte8*>(&m_flags));
    bdUInt16 length = 0;
    ok = ok == bdBytePacker::appendBasicType<bdUInt16>(data, size, offset, &offset, &length);
    ++offset;

    ok = ok == bdBytePacker::appendBasicType<bdSeqNumber>(data, size, offset, &offset, &m_cumulativeAck);
    ok = ok == bdBytePacker::appendBasicType<bdWord>(data, size, offset, &offset, &m_windowCredit);
    numGaps = m_gaps.getSize();
    ok = ok == bdBytePacker::appendBasicType<bdUInt16>(data, size, offset, &offset, &numGaps);
    bdUInt16 tmpZero = 0;
    ok = ok == bdBytePacker::appendBasicType<bdUInt16>(data, size, offset, &offset, &tmpZero);

    for (pos = m_gaps.getHeadPosition(); pos; m_gaps.forward(&pos))
    {
        gap = m_gaps.getAt(pos);

        start = gap->m_start;
        ok = ok == bdBytePacker::appendBasicType<bdUInt16>(data, size, offset, &offset, &start);

        end = gap->m_end;
        ok = ok == bdBytePacker::appendBasicType<bdUInt16>(data, size, offset, &offset, &end);
    }
    return offset;
}

bdBool bdSAckChunk::deserialize(const bdUByte8* const data, const bdUInt size, bdUInt* offset)
{
    Position pos;
    bdUInt16 end;
    bdUInt16 start;
    bdUInt16 numGaps;
    bdUInt bytesRead = *offset;
    bdBool ok = false;

    ok = bdChunk::deserialize(data, size, &bytesRead);
    ok = ok == bdBytePacker::removeBasicType<bdUByte8>(data, size, bytesRead, &bytesRead, reinterpret_cast<bdUByte8*>(&m_flags));

    bdUInt16 length = 0;
    ok = ok == bdBytePacker::removeBasicType<bdUInt16>(data, size, bytesRead, &bytesRead, &length);
    ++bytesRead;

    ok = ok == bdBytePacker::removeBasicType<bdSeqNumber>(data, size, bytesRead, &bytesRead, &m_cumulativeAck);
    ok = ok == bdBytePacker::removeBasicType<bdWord>(data, size, bytesRead, &bytesRead, &m_windowCredit);
    numGaps = 0;
    ok = ok == bdBytePacker::removeBasicType<bdUInt16>(data, size, bytesRead, &bytesRead, &numGaps);
    bytesRead += 2;
    for (bdUInt numGapsRead = 0; numGapsRead < numGaps; ++numGapsRead)
    {
        start = 0;
        ok = ok == bdBytePacker::removeBasicType<bdUInt16>(data, size, bytesRead, &bytesRead, &start);

        end = 0;
        ok = ok == bdBytePacker::removeBasicType<bdUInt16>(data, size, bytesRead, &bytesRead, &end);

        bdSAckChunk::bdGapAckBlock value(start, end);
        m_gaps.addTail(&value);
        bdLogInfo("bdConnection/chunks", "gap ack: %hu-%hu", start, end);
    }
    if (ok)
    {
        *offset = bytesRead;
    }
    return ok;
}

bdUInt bdSAckChunk::getSerializedSize()
{
    return serialize(NULL, NULL);
}

void bdSAckChunk::setCumulativeAck(bdUInt16 cumulativeAck)
{
    m_cumulativeAck = cumulativeAck;
}

void bdSAckChunk::addGap(const bdGapAckBlock* gap)
{
    m_gaps.addTail(gap);
}

void bdSAckChunk::setWindowCredit(bdWord windowCredit)
{
    m_windowCredit = windowCredit;
}

bdLinkedList<bdSAckChunk::bdGapAckBlock>* bdSAckChunk::getGapList()
{
    return &m_gaps;
}

const bdWord bdSAckChunk::getWindowCredit() const
{
    return m_windowCredit;
}

const bdSeqNumber bdSAckChunk::getCumulativeAck() const
{
    return m_cumulativeAck;
}

bdSAckChunk::bdGapAckBlock::bdGapAckBlock(bdSAckChunk::bdGapAckBlock* other)
    : m_start(other->m_start), m_end(other->m_end)
{
}

bdSAckChunk::bdGapAckBlock::bdGapAckBlock()
    : m_start(0), m_end(0)
{
}

bdSAckChunk::bdGapAckBlock::bdGapAckBlock(bdUInt start, bdUInt end)
    : m_start(start), m_end(end)
{
}
