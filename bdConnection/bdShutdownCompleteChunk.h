// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdShutdownCompleteChunk : public bdChunk
{
public:
    enum bdShutdownCompleteFlags : bdInt
    {
        BD_SCC_NO_FLAGS = 0x0,
    };
protected:
    bdShutdownCompleteChunk::bdShutdownCompleteFlags m_flags;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdShutdownCompleteChunk();
    virtual bdUInt serialize(bdUByte8* data, const bdUInt32 size) const;
    virtual bdBool deserialize(const bdUByte8* const data, const bdUInt size, bdUInt& offset);
    virtual bdUInt getSerializedSize();
    const bdShutdownCompleteChunk::bdShutdownCompleteFlags getFlags() const;
};
