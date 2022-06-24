// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdPlatform/bdPlatform.h"

bool bdPlatformThread::createThread(LPTHREAD_START_ROUTINE threadProc, void* args, void** handle, int __formal, unsigned int stackSize)
{
    *handle = CreateThread(0, stackSize, threadProc, args, 4, 0);
    return *handle != 0;
}

bool bdPlatformThread::startThread(void** handle)
{
    return ResumeThread(*handle) != -1;
}

void bdPlatformThread::deleteThread(void** handle)
{
    CloseHandle(*handle);
}
