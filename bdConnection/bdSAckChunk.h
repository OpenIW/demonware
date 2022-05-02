// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdSAckChunk : bdChunk
{
public:
    enum bdSAckFlags : bdInt
    {
        BD_SC_NO_FLAGS = 0x0,
    };
    struct bdGapAckBlock
    {
        bdUInt m_start;
        bdUInt m_end;

        bdGapAckBlock();
        bdGapAckBlock(bdUInt start, bdUInt end);
    };
protected:
    bdSAckFlags m_flags;
    bdSeqNumber m_cumulativeAck;
    bdLinkedList<bdSAckChunk::bdGapAckBlock> m_gaps;
    bdWord m_windowCredit;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdSAckChunk();
    bdSAckChunk(const bdWord windowCredit, bdSAckFlags flags);
    bdUInt serialize(bdUByte8* data, const bdUInt32 size);
    bdBool deserialize(const bdUByte8* const data, const bdUInt size, bdUInt* offset);
    bdUInt getSerializedSize();

    void addGap(const bdSAckChunk::bdGapAckBlock* gap);
    void setCumulativeAck(bdSeqNumber cumulativeAck);
    void setWindowCredit(bdWord windowCredit);

    const bdLinkedList<bdSAckChunk::bdGapAckBlock>* getGapList() const;
    const bdWord getWindowCredit() const;
    const bdSeqNumber getCumulativeAck() const;
};

typedef bdReference<bdSAckChunk> bdSAckChunkRef;
