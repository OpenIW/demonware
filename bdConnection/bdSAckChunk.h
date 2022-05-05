// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdSAckChunk : public bdChunk
{
public:
    enum bdSAckFlags : bdInt
    {
        BD_SC_NO_FLAGS = 0x0,
    };
    class bdGapAckBlock
    {
    public:
        bdUInt m_start;
        bdUInt m_end;

        void operator delete(void* p);
        void* operator new(bdUWord nbytes);
        bdGapAckBlock();
        bdGapAckBlock(bdGapAckBlock* other);
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
    virtual bdUInt serialize(bdUByte8* data, const bdUInt32 size);
    virtual bdBool deserialize(const bdUByte8* const data, const bdUInt size, bdUInt* offset);
    virtual bdUInt getSerializedSize();

    void addGap(const bdSAckChunk::bdGapAckBlock* gap);
    void setCumulativeAck(bdSeqNumber cumulativeAck);
    void setWindowCredit(bdWord windowCredit);

    bdLinkedList<bdSAckChunk::bdGapAckBlock>* getGapList();
    const bdWord getWindowCredit() const;
    const bdSeqNumber getCumulativeAck() const;
};

typedef bdReference<bdSAckChunk> bdSAckChunkRef;
