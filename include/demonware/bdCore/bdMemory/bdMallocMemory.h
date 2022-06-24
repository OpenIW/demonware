// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdMallocMemory
{
public:
    struct bdMemoryChainElement
    {
        bdUInt16 m_magic;
        bdUWord m_size;
        bdBool m_aligned;
        bdMemoryChainElement* m_prev;
        bdMemoryChainElement* m_next;
    };
protected:
    static bdMutex m_mutex;
    static bdMemoryChainElement* m_memoryChain;
    static bdUInt m_allocatedBytes;
    static bdUInt m_numAllocations;
public:
    static void* recordMemory(bdMemoryChainElement* link, const bdUWord size, const bdBool aligned);
    static void eraseMemory(bdMemoryChainElement* link);
    static void* allocate(const bdUWord size);
    static void* alignedAllocate(const bdUWord size, const bdUWord align);
    static void* alignedReallocate(void* p, const bdUWord size, const bdUWord align);
    static void* reallocate(void* p, const bdUWord size);
    static void deallocate(void* p);
    static void alignedDeallocate(void* p);
    static void releaseAllMemory();
};
