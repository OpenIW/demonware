// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdPacket
{
protected:
    bdLinkedList<bdChunkRef> m_chunks;
    bdCommonAddrRef m_me;
    bdUInt32 m_tag;
    bdUInt m_bufferSize;
    bdUInt m_currentSize;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdPacket();
    bdPacket(const bdUInt32 tag, const bdUInt bufferSize);
    bdUInt serialize(bdUByte8* outBuffer, const bdUInt outSize);
    bdBool deserialize(const bdUByte8* inData, const bdUInt inSize);
    bdBool addChunk(bdChunkRef chunk);
    bdBool getNextChunk(bdChunkRef& chunk);

    const bdBool isEmpty() const;
    const bdUInt32 getVerificationTag() const;
};
