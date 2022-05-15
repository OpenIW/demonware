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
    : m_handle()
{
}

void bdSemaphore::release()
{
}

bool bdSemaphore::wait()
{
    return false;
}

void bdSemaphore::destroy()
{
}
