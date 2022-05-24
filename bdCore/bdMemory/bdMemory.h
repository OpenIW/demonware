// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

template<typename T>
inline T* bdAllocate(const bdUWord n)
{
    return reinterpret_cast<T*>(bdMemory::allocate(n));
}

template<typename T>
inline void bdDeallocate(T* p)
{
    bdMemory::deallocate(p);
}

template<typename T>
inline T* bdReallocate(T* p, const bdUWord n)
{
    return reinterpret_cast<T*>(bdMemory::reallocate(p, n));
}

class bdMemory
{
public:
    typedef void* (*bdAllocateFunc)(const bdUWord);
    typedef void (*bdDeallocateFunc)(void*);
    typedef void* (*bdReallocateFunc)(void*, const bdUWord);
    typedef void* (*bdAlignedAllocateFunc)(const bdUWord, const bdUWord);
    typedef void (*bdAlignedDeallocateFunc)(void*);
    typedef void* (*bdAlignedReallocateFunc)(void*, const bdUWord, const bdUWord);

    static void setAllocateFunc(const bdMemory::bdAllocateFunc allocator);
    static void setDeallocateFunc(const bdMemory::bdDeallocateFunc deallocator);
    static void setReallocateFunc(const bdMemory::bdReallocateFunc reallocator);
    static void setAlignedAllocateFunc(const bdMemory::bdAlignedAllocateFunc alignedAllocator);
    static void setAlignedDeallocateFunc(const bdMemory::bdAlignedDeallocateFunc alignedDeallocator);
    static void setAlignedReallocateFunc(const bdMemory::bdAlignedReallocateFunc alignedReallocator);
    static bdMemory::bdAllocateFunc getAllocateFunc();
    static bdMemory::bdDeallocateFunc getDeallocateFunc();
    static bdMemory::bdReallocateFunc getReallocateFunc();
    static bdMemory::bdAlignedAllocateFunc getAlignedAllocateFunc();
    static bdMemory::bdAlignedDeallocateFunc getAlignedDeallocateFunc();
    static bdMemory::bdAlignedReallocateFunc getAlignedReallocateFunc();
    static void* allocate(const bdUWord size);
    static void deallocate(void* p);
    static void* reallocate(void* p, const bdUWord size);
    static void* alignedAllocate(const bdUWord size, const bdUWord align);
    static void alignedDeallocate(void* p);
    static void* alignedReallocate(void* p, const bdUWord size, const bdUWord align);
protected:
    static bdMemory::bdAllocateFunc m_allocateFunc;
    static bdMemory::bdDeallocateFunc m_deallocateFunc;
    static bdMemory::bdReallocateFunc m_reallocateFunc;
    static bdMemory::bdAlignedAllocateFunc m_alignedAllocateFunc;
    static bdMemory::bdAlignedDeallocateFunc m_alignedDeallocateFunc;
    static bdMemory::bdAlignedReallocateFunc m_alignedReallocateFunc;
private:
    bdMemory();
};

void* operator new(bdUWord nbytes, void* p);
