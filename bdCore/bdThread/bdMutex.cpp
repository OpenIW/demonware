// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdCore/bdCore.h"

bdMutex::bdMutex()
{
    m_handle = bdPlatformMutex::createMutex();
}

bdMutex::~bdMutex()
{
    bdPlatformMutex::destroy(&m_handle);
}

void bdMutex::lock()
{
    bdPlatformMutex::lock(&m_handle);
}

void bdMutex::unlock()
{
    bdPlatformMutex::unlock(&m_handle);
}
