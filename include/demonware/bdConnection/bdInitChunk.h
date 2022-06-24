// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdInitChunk : public bdChunk
{
public:
    enum bdInitChunkFlags : bdInt
    {
        BD_IC_NO_FLAGS_0 = 0x0,
    };
protected:
    bdUInt32 m_initTag;
    bdInitChunkFlags m_flags;
    bdSecurityID m_id;
    bdByteBufferRef m_theirKey;
    bdWord m_windowCredit;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdInitChunk();
    bdInitChunk(bdUInt32 initTag, const bdWord windowCredit);
    virtual bdUInt serialize(bdUByte8* data, const bdUInt32 size) const;
    virtual bdBool deserialize(const bdUByte8* const data, const bdUInt size, bdUInt& offset);
    virtual bdUInt getSerializedSize();

    const bdWord getWindowCredit() const;
    const bdUInt32 getInitTag() const;
};

typedef bdReference<bdInitChunk> bdInitChunkRef;
