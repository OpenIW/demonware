// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdTaskByteBuffer::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdTaskByteBuffer::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdTaskByteBuffer::bdTaskByteBuffer(const bdUInt size, bdBool isTypeChecked)
    : bdByteBuffer(0, isTypeChecked), m_taskData(NULL), m_taskDataSize(0), m_validHeaderSize(0), m_paddingSize(0)
{
    setupTaskData(size);
}

bdTaskByteBuffer::bdTaskByteBuffer(const bdUByte8* bytes, const bdUInt size, bdBool isTypeChecked)
    : bdByteBuffer(NULL, 0, isTypeChecked), m_taskData(NULL), m_taskDataSize(0), m_validHeaderSize(0), m_paddingSize(0)
{
    if (bytes)
    {
        setupTaskData(size);
        bdMemcpy(m_data, bytes, size);
    }
}

bdTaskByteBuffer::~bdTaskByteBuffer()
{
    if (m_taskData)
    {
        bdDeallocate<bdUByte8>(m_taskData);
    }
    m_taskData = NULL;
    m_data = NULL;
    m_readPtr = NULL;
    m_writePtr = NULL;
}

void bdTaskByteBuffer::setupTaskData(bdUInt size)
{
    if (size)
    {
        m_paddingSize = (size + 11) & 0xFFFFFFF8;
        m_paddingSize -= size;
        m_taskDataSize = size + m_paddingSize + 14;
        m_taskData = bdAllocate<bdUByte8>(m_taskDataSize);
        m_data = m_taskData + 14;
        m_validHeaderSize = 14;
        m_size = size;
    }
    m_writePtr = m_data;
    m_readPtr = m_data;
}

void bdTaskByteBuffer::allocateBuffer()
{
    if (m_taskData)
    {
        bdLogWarn("core/taskbytebuffer", "Buffer already allocated.");
        return;
    }
    setupTaskData(m_size);
}

bdUInt bdTaskByteBuffer::getHeaderSize() const
{
    return m_validHeaderSize;
}

void bdTaskByteBuffer::setHeaderSize(bdUInt size)
{
    m_validHeaderSize = size;
}

bdUByte8* bdTaskByteBuffer::getHeaderStart() const
{
    if (m_taskData)
    {
        return &m_taskData[14 - m_validHeaderSize];
    }
    return NULL;
}

bdUInt bdTaskByteBuffer::getPaddingSize() const
{
    return m_paddingSize;
}

bdUByte8* bdTaskByteBuffer::getStartOfTaskDataBuffer() const
{
    return m_taskData;
}

bdUInt bdTaskByteBuffer::getSizeOfTaskDataBuffer() const
{
    return m_taskDataSize;
}
