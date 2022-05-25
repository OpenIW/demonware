// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdHMacSHA1 : bdHMac
{
protected:
    hmac_state m_state;
public:
    bdHMacSHA1();
    bdHMacSHA1(const bdUByte8* const key, const bdUInt keySize);
    ~bdHMacSHA1();
    void* operator new(bdUWord nbytes);
    void* operator new(bdUWord __needed, void* p);
    void operator delete(void* p);
    bdBool process(const bdUByte8* const data, const bdUInt length);
    bdBool getData(bdUByte8* dst, bdUInt& length);
};
