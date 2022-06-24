// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdCore/bdCore.h"

bdBool bdCore::m_initialized = false;

void bdCore::init(const bdBool defaultMemoryFunctions)
{
    if (m_initialized)
    {
        bdLogWarn("core", "init() has been called twice without an intermediate quit()");
        return;
    }
    if (defaultMemoryFunctions)
    {
        bdMemory::setAllocateFunc(bdMallocMemory::allocate);
        bdMemory::setAlignedAllocateFunc(bdMallocMemory::alignedAllocate);
        bdMemory::setDeallocateFunc(bdMallocMemory::deallocate);
        bdMemory::setAlignedDeallocateFunc(bdMallocMemory::alignedDeallocate);
        bdMemory::setReallocateFunc(bdMallocMemory::reallocate);
        bdMemory::setAlignedReallocateFunc(bdMallocMemory::alignedReallocate);
    }
    m_initialized = true;
}

void bdCore::quit()
{
    if (m_initialized)
    {
        bdSingleton<bdSingletonRegistryImpl>::getInstance()->cleanUp();
        bdMemory::setAllocateFunc(NULL);
        bdMemory::setAlignedAllocateFunc(NULL);
        bdMemory::setDeallocateFunc(NULL);
        bdMemory::setAlignedDeallocateFunc(NULL);
        bdMemory::setReallocateFunc(NULL);
        bdMemory::setAlignedReallocateFunc(NULL);
        m_initialized = false;
    }
    else
    {
        bdLogWarn("core", "quit() has been called twice without an intermediate init()");
    }
}
