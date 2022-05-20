// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdCore/bdCore.h"

void bdSemaphore::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdSemaphore::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdSemaphore::bdSemaphore(LONG lInitialCount, LONG lMaximumCount)
    : m_handle(bdPlatformSemaphore::createSemaphore(lInitialCount, lMaximumCount))
{
}

void bdSemaphore::release()
{
    bdPlatformSemaphore::release(&m_handle);
}

bdBool bdSemaphore::wait()
{
    return bdPlatformSemaphore::wait(&m_handle);
}

void bdSemaphore::destroy()
{
    bdPlatformSemaphore::destroy(&m_handle);
    if (this)
    {
        delete this;
    }
}
