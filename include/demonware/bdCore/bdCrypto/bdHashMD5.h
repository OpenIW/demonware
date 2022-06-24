// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdHashMD5 : bdHash
{
public:
    bdHashMD5();
    ~bdHashMD5();
    void* operator new(bdUWord nbytes);
    void* operator new(bdUWord __needed, void* p);
    void operator delete(void* p);
    bdBool hash(const bdUByte8* data, const bdUInt dataSize, bdUByte8* result, bdUInt* resultSize);
};