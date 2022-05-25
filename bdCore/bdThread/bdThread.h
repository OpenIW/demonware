// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdThreadArgs
{
public:
    void* m_args;
    class bdRunnable* m_runnable;
    class bdThread* m_thread;
};

class bdThread
{
public:
    class bdRunnable* m_runnable;
    bdThreadArgs m_threadArgs;
    void* m_handle;
    bool m_isRunning;
    unsigned int m_stackSize;
    unsigned int m_priority;

    bdThread(class bdRunnable* runnable, int priority, unsigned int stackSize);
    ~bdThread();
    void join();
    void cleanup();
    void wakeupJoiningThread();
    void setThreadName(const char* name);
    static int threadProc(bdThreadArgs* args);
    bdBool start(const void* args, const bdUWord size);
};
