// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdUploadInterceptor
{
public:
    void operator delete(void* p) { bdMemory::deallocate(p); };
    virtual ~bdUploadInterceptor() {};
    virtual bdUInt handleUpload(void*, bdUInt, bdUInt) { return 0; };
};