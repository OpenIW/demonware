// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#define BD_AES128_KEY_SIZE 16
#define BD_AES192_KEY_SIZE 24
#define BD_AES256_KEY_SIZE 32

class bdCypherAES : bdCypher
{
public:
    bdUInt32 m_keySize;
    symmetric_CBC m_cbc;

    bdCypherAES();
    ~bdCypherAES();
    void* operator new(bdUWord nbytes);
    void* operator new(bdUWord __needed, void* p);
    void operator delete(void* p);
    bdBool init(const bdUByte8* key, bdUInt keySize);
    bdBool decrypt(const bdUByte8* iv, const bdUByte8* cypherText, bdUByte8* plainText, bdUInt size);
    bdBool encrypt(const bdUByte8* iv, const bdUByte8* plainText, bdUByte8* cypherText, bdUInt size);
};