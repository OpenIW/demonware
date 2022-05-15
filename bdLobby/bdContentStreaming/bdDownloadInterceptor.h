// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdDownloadInterceptor
{
public:
    void operator delete(void* p) { bdMemory::deallocate(p); };
    virtual ~bdDownloadInterceptor() {};
    virtual bdUInt handleDownload(void*, bdUInt) { return 0; };
};