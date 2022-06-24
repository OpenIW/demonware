// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdPlatform/bdPlatform.h"

HANDLE bdPlatformMutex::createMutex()
{
    return CreateMutex(0, 0, 0);
}

void bdPlatformMutex::unlock(void** handle)
{
    ReleaseMutex(*handle);
}

void bdPlatformMutex::destroy(void** handle)
{
    ReleaseMutex(*handle);
    CloseHandle(*handle);
}

void bdPlatformMutex::lock(void** handle)
{
    WaitForSingleObject(*handle, -1);
}
