// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdShutdownChunk : public bdChunk
{
public:
    enum bdShutdownFlags : bdInt
    {
        BD_SC_NO_FLAGS_0 = 0x0,
    };
protected:
    bdShutdownChunk::bdShutdownFlags m_flags;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdShutdownChunk();
    virtual bdUInt serialize(bdUByte8* data, const bdUInt32 size);
    virtual bdBool deserialize(const bdUByte8* const data, const bdUInt size, bdUInt* offset);
    virtual bdUInt getSerializedSize();
    const bdShutdownChunk::bdShutdownFlags getFlags() const;
};