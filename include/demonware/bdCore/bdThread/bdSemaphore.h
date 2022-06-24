// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdSemaphore
{
protected:
    HANDLE m_handle;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdSemaphore(LONG lInitialCount, LONG lMaximumCount);
    void release();
    bdBool wait();
    void destroy();
};