// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdPreCopyResult : public bdTaskResult
{
public:
    bdURL* m_source;
    bdNChar8 m_destination[384];
    bdUInt m_fileSize;

    void operator delete(void* p);
    bdPreCopyResult();
    ~bdPreCopyResult();
    virtual bdBool deserialize(bdByteBufferRef buffer);
    virtual bdUInt sizeOf();
protected:
    void reset();
};