// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdCore/bdCore.h"

#define BD_MEMORY_MAGIC 0xBDBD

bdMutex bdMallocMemory::m_mutex;
bdMallocMemory::bdMemoryChainElement* bdMallocMemory::m_memoryChain;
bdUInt bdMallocMemory::m_allocatedBytes = 0;
bdUInt bdMallocMemory::m_numAllocations = 0;

void* bdMallocMemory::recordMemory(bdMemoryChainElement* link, const bdUWord size, const bdBool aligned)
{
    if (!link)
    {
        return NULL;
    }
    m_mutex.lock();
    link->m_magic = BD_MEMORY_MAGIC;
    link->m_size = size;
    link->m_aligned = aligned;
    link->m_next = m_memoryChain;
    link->m_prev = NULL;
    if (m_memoryChain)
    {
        m_memoryChain->m_prev = link;
    }
    m_memoryChain = link;
    m_allocatedBytes += size;
    ++m_numAllocations;
    m_mutex.unlock();
    return link + 1;
}

void bdMallocMemory::eraseMemory(bdMemoryChainElement* link)
{
    m_mutex.lock();
    if (link->m_magic != BD_MEMORY_MAGIC)
    {
        m_mutex.unlock();
        bdLogError("mallocmemory", " BD_MEMORY_MAGIC is incorrect.");
        m_mutex.lock();
    }
    if (link->m_prev)
    {
        link->m_prev->m_next = link->m_next;
    }
    else
    {
        m_memoryChain = link->m_next;
    }

    if (link->m_next)
    {
        link->m_next->m_prev = link->m_prev;
    }
    m_allocatedBytes -= link->m_size;
    --m_numAllocations;
    m_mutex.unlock();
}

void bdMallocMemory::releaseAllMemory()
{
    bdMallocMemory::bdMemoryChainElement* link;

    m_mutex.lock();
    while (m_memoryChain)
    {
        link = m_memoryChain;
        eraseMemory(m_memoryChain);
        bdAlignedOffsetFree(link);
    }
    m_mutex.unlock();
}

void* bdMallocMemory::allocate(const bdUWord size)
{
    bdMemoryChainElement* link =
        reinterpret_cast<bdMemoryChainElement*>(bdAlignedOffsetMalloc(size + sizeof(bdMemoryChainElement), 8, sizeof(bdMemoryChainElement)));
    return recordMemory(link, size, false);
}

void* bdMallocMemory::alignedAllocate(const bdUWord size, const bdUWord align)
{
    bdMemoryChainElement* link =
        reinterpret_cast<bdMemoryChainElement*>(bdAlignedOffsetMalloc(size + sizeof(bdMemoryChainElement), align, sizeof(bdMemoryChainElement)));
    return recordMemory(link, size, true);
}

void* bdMallocMemory::alignedReallocate(void* p, const bdUWord size, const bdUWord align)
{
    if (!p)
    {
        return alignedAllocate(size, align);
    }
    bdUInt origSize = reinterpret_cast<bdMemoryChainElement*>((char*)p - sizeof(bdMemoryChainElement))->m_size;
    eraseMemory(reinterpret_cast<bdMemoryChainElement*>((char*)p - sizeof(bdMemoryChainElement)));
    bdMemoryChainElement* link =
        reinterpret_cast<bdMemoryChainElement*>(
            bdAlignedOffsetRealloc((char*)p - sizeof(bdMemoryChainElement), origSize, size + sizeof(bdMemoryChainElement), align, sizeof(bdMemoryChainElement)));
    return recordMemory(link, size, true);
}

void* bdMallocMemory::reallocate(void* p, const bdUWord size)
{
    if (!p)
    {
        return allocate(size);
    }
    bdUInt origSize = reinterpret_cast<bdMemoryChainElement*>((char*)p - sizeof(bdMemoryChainElement))->m_size;
    eraseMemory(reinterpret_cast<bdMemoryChainElement*>((char*)p - sizeof(bdMemoryChainElement)));
    bdMemoryChainElement* link =
        reinterpret_cast<bdMemoryChainElement*>(
            bdAlignedOffsetRealloc((char*)p - sizeof(bdMemoryChainElement), origSize, size + sizeof(bdMemoryChainElement), 8, sizeof(bdMemoryChainElement)));
    return recordMemory(link, size, true);
}

void bdMallocMemory::deallocate(void* p)
{
    if (!p)
    {
        return;
    }

    bdMemoryChainElement* link = reinterpret_cast<bdMemoryChainElement*>((char*)p - sizeof(bdMemoryChainElement));
    bdAssert(link->m_aligned == false,
        "Memory block allocated as aligned but is being deallocated with bdMallocMemory::deallocate. Use bdMallocMemory::alignedDeallocate instead.");
    eraseMemory(link);
    bdAlignedOffsetFree(link);
}

void bdMallocMemory::alignedDeallocate(void* p)
{
    if (!p)
    {
        return;
    }

    bdMemoryChainElement* link = reinterpret_cast<bdMemoryChainElement*>((char*)p - sizeof(bdMemoryChainElement));
    bdAssert(link->m_aligned == true,
        "Memory block allocated unaligned but is being deallocated with bdMallocMemory::alignedDeallocate. Use bdMallocMemory::deallocate instead.");
    eraseMemory(link);
    bdAlignedOffsetFree(link);
}
