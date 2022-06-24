// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdFileData : public bdTaskResult
{
public:
    void* m_fileData;
    bdUInt m_fileSize;

    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdFileData(void* fileData, bdUInt fileSize);
    virtual bdBool deserialize(bdByteBufferRef buffer);
    virtual bdUInt sizeOf();
};