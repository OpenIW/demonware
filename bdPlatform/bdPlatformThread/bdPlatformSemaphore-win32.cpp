// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdPlatform/bdPlatform.h"

HANDLE bdPlatformSemaphore::createSemaphore(LONG lInitialCount, LONG lMaximumCount)
{
    return CreateSemaphore(NULL, lInitialCount, lMaximumCount, NULL);
}

void bdPlatformSemaphore::destroy(HANDLE* h)
{
    ReleaseSemaphore(*h, 1, 0);
    CloseHandle(*h);
}

void bdPlatformSemaphore::release(HANDLE* h)
{
    ReleaseSemaphore(*h, 1, 0);
}

bdBool bdPlatformSemaphore::wait(HANDLE* h)
{
    return WaitForSingleObject(*h, -1) == 0;
}
