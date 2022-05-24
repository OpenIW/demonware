// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdCore/bdCore.h"

bdMemory::bdAllocateFunc bdMemory::m_allocateFunc = NULL;
bdMemory::bdDeallocateFunc bdMemory::m_deallocateFunc = NULL;
bdMemory::bdReallocateFunc bdMemory::m_reallocateFunc = NULL;
bdMemory::bdAlignedAllocateFunc bdMemory::m_alignedAllocateFunc = NULL;
bdMemory::bdAlignedDeallocateFunc bdMemory::m_alignedDeallocateFunc = NULL;
bdMemory::bdAlignedReallocateFunc bdMemory::m_alignedReallocateFunc = NULL;

static bdMutex g_MemoryThreadLock;

void bdMemory::setAllocateFunc(const bdMemory::bdAllocateFunc allocator)
{
    m_allocateFunc = allocator;
}

void bdMemory::setDeallocateFunc(const bdMemory::bdDeallocateFunc deallocator)
{
    m_deallocateFunc = deallocator;
}

void bdMemory::setReallocateFunc(const bdMemory::bdReallocateFunc reallocator)
{
    m_reallocateFunc = reallocator;
}

void bdMemory::setAlignedAllocateFunc(const bdMemory::bdAlignedAllocateFunc alignedAllocator)
{
    m_alignedAllocateFunc = alignedAllocator;
}

void bdMemory::setAlignedDeallocateFunc(const bdMemory::bdAlignedDeallocateFunc alignedDeallocator)
{
    m_alignedDeallocateFunc = alignedDeallocator;
}

void bdMemory::setAlignedReallocateFunc(const bdMemory::bdAlignedReallocateFunc alignedReallocator)
{
    m_alignedReallocateFunc = alignedReallocator;
}

bdMemory::bdAllocateFunc bdMemory::getAllocateFunc()
{
    return m_allocateFunc;
}

bdMemory::bdDeallocateFunc bdMemory::getDeallocateFunc()
{
    return m_deallocateFunc;
}

bdMemory::bdReallocateFunc bdMemory::getReallocateFunc()
{
    return m_reallocateFunc;
}

bdMemory::bdAlignedAllocateFunc bdMemory::getAlignedAllocateFunc()
{
    return m_alignedAllocateFunc;
}

bdMemory::bdAlignedDeallocateFunc bdMemory::getAlignedDeallocateFunc()
{
    return m_alignedDeallocateFunc;
}

bdMemory::bdAlignedReallocateFunc bdMemory::getAlignedReallocateFunc()
{
    return m_alignedReallocateFunc;
}

void* bdMemory::allocate(const bdUWord size)
{
    void* block = NULL;
    if (m_allocateFunc)
    {
        g_MemoryThreadLock.lock();
        block = m_allocateFunc(size);
        g_MemoryThreadLock.unlock();
        if (!block)
        {
            DebugBreak();
        }
    }
    return block;
}

void bdMemory::deallocate(void* p)
{
    if (m_deallocateFunc)
    {
        g_MemoryThreadLock.lock();
        m_deallocateFunc(p);
        g_MemoryThreadLock.unlock();
    }
}

void* bdMemory::reallocate(void* p, const bdUWord size)
{
    void* block = NULL;
    if (m_reallocateFunc)
    {
        g_MemoryThreadLock.lock();
        block = m_reallocateFunc(p, size);
        g_MemoryThreadLock.unlock();
        if (!block)
        {
            DebugBreak();
        }
    }
    return block;
}

void* bdMemory::alignedAllocate(const bdUWord size, const bdUWord align)
{
    void* block = NULL;
    if (m_alignedAllocateFunc)
    {
        g_MemoryThreadLock.lock();
        block = m_alignedAllocateFunc(size, align);
        g_MemoryThreadLock.unlock();
        if (!block)
        {
            DebugBreak();
        }
    }
    return block;
}

void bdMemory::alignedDeallocate(void* p)
{
    if (m_alignedDeallocateFunc)
    {
        g_MemoryThreadLock.lock();
        m_alignedDeallocateFunc(p);
        g_MemoryThreadLock.unlock();
    }
}

void* bdMemory::alignedReallocate(void* p, const bdUWord size, const bdUWord align)
{
    void* block = NULL;
    if (m_alignedReallocateFunc)
    {
        g_MemoryThreadLock.lock();
        block = m_alignedReallocateFunc(p, size, align);
        g_MemoryThreadLock.unlock();
        if (!block)
        {
            DebugBreak();
        }
    }
    return block;
}

bdMemory::bdMemory()
{
}

void* operator new(bdUWord nbytes, void* p)
{
    return p;
}
