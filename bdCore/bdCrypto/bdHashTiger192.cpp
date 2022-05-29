// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdCore/bdCore.h"

bdHashTiger192::bdHashTiger192() : bdHash(BD_HASH_TIGER192, 24u)
{
    if (register_hash(&tiger_desc) == -1)
    {
        //bdLog(BD_LOG_ERROR, "Unable to register hash.");
    }
}

bdHashTiger192::~bdHashTiger192()
{
}

void* bdHashTiger192::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

void* bdHashTiger192::operator new(bdUWord __needed, void* p)
{
    return p;
}

void bdHashTiger192::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

bdBool bdHashTiger192::hash(const bdUByte8* data, const bdUInt dataSize, bdUByte8* result, bdUInt& resultSize)
{
    bdInt hash;
    bdInt error;
    bdULong resultSz;

    if (resultSize > 24)
    {
        bdLogWarn("hashtiger192", "Tiger hash only requires result buffer of 24 bytes");
    }
    resultSz = resultSize;
    hash = find_hash("tiger");
    error = hash_memory(hash, data, dataSize, result, &resultSz);

    if (error == 6)
    {
        bdLogError("bdCrypto/hash", "Unable to create tiger hash of less than 24 bytes in length!");
        return false;
    }
    else if (error)
    {
        bdLogError("hashtiger192", "Unable to create hash.");
        return false;
    }
    resultSize = resultSz;
    return true;
}
