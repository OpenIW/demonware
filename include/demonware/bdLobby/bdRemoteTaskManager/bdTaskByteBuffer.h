// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdTaskByteBuffer : public bdByteBuffer
{
protected:
    bdUByte8* m_taskData;
    bdUInt m_taskDataSize;
    bdUInt m_validHeaderSize;
    bdUInt m_paddingSize;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdTaskByteBuffer(const bdUInt size, bdBool isTypeChecked);
    bdTaskByteBuffer(const bdUByte8* bytes, const bdUInt size, bdBool isTypeChecked);
    virtual ~bdTaskByteBuffer();
    void setupTaskData(bdUInt size);
    virtual void allocateBuffer();
    bdUInt getHeaderSize() const;
    void setHeaderSize(bdUInt size);
    bdUByte8* getHeaderStart() const;
    bdUInt getPaddingSize() const;
    bdUByte8* getStartOfTaskDataBuffer() const;
    bdUInt getSizeOfTaskDataBuffer() const;
};

typedef bdReference<bdTaskByteBuffer> bdTaskByteBufferRef;
