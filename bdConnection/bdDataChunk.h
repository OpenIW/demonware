// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdDataChunk : bdChunk
{
public:
    enum bdDataFlags : bdInt
    {
        BD_DC_NO_FLAGS = 0x0,
        BD_DC_UNRELIABLE = 0x1,
        BD_DC_ENC_DATA = 0x2,
        BD_DC_UNENC_DATA = 0x4,
    };
protected:
    bdMessageRef m_message;
    bdUByte8 m_flags;
    bdSeqNumber m_seqNum;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdDataChunk();
    bdDataChunk(bdMessageRef data, const bdDataChunk::bdDataFlags flags);
    bdUInt serialize(bdUByte8* data, const bdUInt32 size);
    bdUInt serializeUnencrypted(bdUByte8* data, const bdUInt32 size);
    bdBool deserialize(const bdUByte8* const data, const bdUInt size, bdUInt* offset);
    bdBool deserialize(const bdUByte8* const data, const bdUInt size, bdUInt* offset, const bdUByte8* const unencData, bdUInt* unencOffset);
    bdUInt getSerializedSize();
    void setSequenceNUmber(bdSeqNumber seqNum);

    const bdSeqNumber getSequenceNumber() const;
    const bdMessageRef getMessage() const;
    const bdUByte8 getFlags() const;
};

typedef bdReference<bdDataChunk> bdDataChunkRef;
