// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdShutdownAckChunk : bdChunk
{
public:
    enum bdShutdownAckFlags : bdInt
    {
        BD_SA_NO_FLAGS = 0x0,
    };
protected:
    bdShutdownAckChunk::bdShutdownAckFlags m_flags;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdShutdownAckChunk();
    bdUInt serialize(bdUByte8* data, const bdUInt32 size);
    bdBool deserialize(const bdUByte8* const data, const bdUInt size, bdUInt* offset);
    bdUInt getSerializedSize();
    const bdShutdownAckChunk::bdShutdownAckFlags getFlags() const;
};
