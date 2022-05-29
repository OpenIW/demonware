// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdCore/bdCore.h"

bdHashMD5::bdHashMD5() : bdHash(BD_HASH_MD5, 16u)
{
    if (register_hash(&md5_desc) == -1)
    {
        //bdLog(BD_LOG_ERROR, "Unable to register hash.");
    }
}

bdHashMD5::~bdHashMD5()
{
}

void* bdHashMD5::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

void* bdHashMD5::operator new(bdUWord __needed, void* p)
{
    return p;
}

void bdHashMD5::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

bdBool bdHashMD5::hash(const bdUByte8* data, const bdUInt dataSize, bdUByte8* result, bdUInt* resultSize)
{
    int hash;
    int error;
    unsigned long resultSz;

    if (*resultSize > 16)
    {
        //bdLog(BD_LOG_WARNING, "MD5 hash only requires result buffer of 16 bytes");
    }
    resultSz = *resultSize;
    hash = find_hash("md5");
    error = hash_memory(hash, data, dataSize, result, &resultSz);

    if (error == 6)
    {
        //bdLog(BD_LOG_ERROR, "Unable to create md5 hash of less than 16 bytes in length!");
        return false;
    }
    else if (error)
    {
        //bdLog(BD_LOG_ERROR, "Unable to create hash.");
        return false;
    }
    *resultSize = resultSz;
    return true;
}
