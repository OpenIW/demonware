// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdCypher3Des : public bdCypher
{
protected:
    symmetric_CBC m_cbc;
public:
    bdCypher3Des();
    ~bdCypher3Des();
    void* operator new(bdUWord nbytes);
    void* operator new(bdUWord __needed, void* p);
    void operator delete(void* p);
    bdBool init(const bdUByte8* key, bdUInt keySize);
    bdBool decrypt(const bdUByte8* iv, const bdUByte8* cypherText, bdUByte8* plainText, bdUInt size);
    bdBool encrypt(const bdUByte8* iv, const bdUByte8* plainText, bdUByte8* cypherText, bdUInt size);
};