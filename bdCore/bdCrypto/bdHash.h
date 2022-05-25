// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#define BD_HASH_DIGEST_SIZE_MAX 32

enum bdHashAlgorithms : __int32
{
    BD_HASH_SHA256 = 0x0,
    BD_HASH_TIGER192 = 0x1,
    BD_HASH_SHA1 = 0x2,
    BD_HASH_MD5 = 0x3,
    BD_MAX_HASH_ALGORITHMS = 0x4,
};

class bdHash
{
protected:
    bdHashAlgorithms m_type;
    bdUInt32 m_digestSize;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdHash();
    bdHash(bdHashAlgorithms type, bdUInt digestSize);
    ~bdHash();
};

class bdHashTiger192 : bdHash
{
public:
    bdHashTiger192();
    ~bdHashTiger192();
    void* operator new(bdUWord nbytes);
    void* operator new(bdUWord __needed, void* p);
    void operator delete(void* p);
    bdBool hash(const bdUByte8* data, const bdUInt dataSize, bdUByte8* result, bdUInt& resultSize);
};
