// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdCore/bdCore.h"

bdThread::bdThread(bdRunnable* runnable, int priority, unsigned int stackSize)
{
    m_handle = NULL;
    m_runnable = runnable;
    m_threadArgs.m_args = 0;
    m_threadArgs.m_runnable = 0;
    m_isRunning = 0;
    m_stackSize = stackSize;
    m_priority = priority;
    m_threadArgs.m_runnable = m_runnable;
    m_threadArgs.m_thread = this;
}

bdThread::~bdThread()
{
    if (m_threadArgs.m_args)
    {
        bdMemory::deallocate(m_threadArgs.m_args);
    }
    bdPlatformThread::deleteThread(&m_handle);
    m_runnable = 0;
}

void bdThread::join()
{
    bdPlatformMutex::lock(&m_handle);
    if (m_threadArgs.m_args)
    {
        bdMemory::deallocate(m_threadArgs.m_args);
        m_threadArgs.m_args = 0;
    }
}

void bdThread::cleanup()
{
    if (m_threadArgs.m_args)
    {
        bdMemory::deallocate(m_threadArgs.m_args);
        m_threadArgs.m_args = NULL;
    }
    if (this)
    {
        delete this;
    }
}

void bdThread::wakeupJoiningThread()
{
    m_isRunning = 0;
}

void bdThread::setThreadName(const char* name)
{
}

int bdThread::threadProc(bdThreadArgs* args)
{
    unsigned int run;

    run = args->m_runnable->run(args->m_args);
    args->m_thread->wakeupJoiningThread();
    return run;
}

bdBool bdThread::start(const void* args, const bdUWord size)
{
    bool isNewThread;

    isNewThread = m_runnable && !m_isRunning;
    m_threadArgs.m_args = 0;
    if (isNewThread && size)
    {
        m_threadArgs.m_args = bdMemory::allocate(size);
        memcpy(m_threadArgs.m_args, args, size);
    }
    if (isNewThread)
    {
        m_isRunning = 1;
        m_runnable->start();
        return bdPlatformThread::createThread((LPTHREAD_START_ROUTINE)threadProc, &m_threadArgs, &m_handle, m_priority, m_stackSize) &&
            bdPlatformThread::startThread(&m_handle);
    }
    return isNewThread;
}
