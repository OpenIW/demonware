// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdPlatformSemaphore
{
public:
    static HANDLE createSemaphore(LONG lInitialCount, LONG lMaximumCount);
    static void destroy(HANDLE* h);
    static void release(HANDLE* h);
    static bdBool wait(HANDLE* h);
};