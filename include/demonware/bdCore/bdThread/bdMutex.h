// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdMutex
{
public:
    bdMutex();
    ~bdMutex();
    void lock();
    void unlock();
protected:
    void* m_handle;
};
