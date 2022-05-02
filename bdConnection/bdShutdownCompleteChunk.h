// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdShutdownCompleteChunk : bdChunk
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
    bdUInt serialize(bdUByte8* data, const bdUInt32 size);
    bdBool deserialize(const bdUByte8* const data, const bdUInt size, bdUInt* offset);
    bdUInt getSerializedSize();
    const bdShutdownCompleteChunk::bdShutdownCompleteFlags getFlags() const;
};