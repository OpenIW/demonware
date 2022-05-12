// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdCore/bdCore.h"

void bdCryptoUtils::calculateInitialVector(const bdUInt32 seed, bdUByte8* iv)
{
    bdBool appended;
    bdBool success;
    bdUInt32 tmp;
    bdUInt IVSize;
    bdHashTiger192 tigerHash;
    bdUByte8 seedBuffer[4];

    IVSize = 24;
    tmp = 0;
    appended = bdBytePacker::appendBasicType<bdUInt32>(reinterpret_cast<void*>(seedBuffer), 4u, 0, &tmp, const_cast<bdUInt32*>(&seed));
    bdAssert(appended, "Failed to serialize ivseed");
    success = tigerHash.hash(seedBuffer, 4u, iv, &IVSize);
    bdAssert(success, "Hash function failed.");
}

bdBool bdCryptoUtils::decrypt(const void* key, const void* initialVector, const void* source, void* dest, const bdUInt size)
{
    bdBool decryptSuccess = false;

    if ((size & 7) != 0)
    {
        //bdLogError("Source data length must be a multiple of 8 for 3DES");
        return false;
    }
    bdCypher3Des cypher;
    cypher.init(reinterpret_cast<const bdUByte8*>(key), 24);
    decryptSuccess = cypher.decrypt(reinterpret_cast<const bdUByte8*>(initialVector), reinterpret_cast<const bdUByte8*>(source), reinterpret_cast<bdUByte8*>(dest), size);
    return decryptSuccess;
}

bdBool bdCryptoUtils::decryptAES(const void* key, const void* initialVector, const void* source, void* dest, bdUInt size, bdUInt keySize)
{
    bdBool decryptSuccess = false;

    bdAssert((size & 0xF) == 0, "Source data length must be a multiple of 16 for AES");
    if ((size & 0xF) == 0)
    {
        //bdLogError("Source data length must be a multiple of 16 for AES");
        return false;
    }
    bdCypherAES cypher;
    bdAssert((keySize == BD_AES128_KEY_SIZE || keySize == BD_AES192_KEY_SIZE || keySize == BD_AES256_KEY_SIZE), "Cannot decryptAES with key of size[%u] (Key must be 16, 24 or 32 bytes)");
    cypher.init(reinterpret_cast<const bdUByte8*>(key), keySize);
    decryptSuccess = cypher.decrypt(reinterpret_cast<const bdUByte8*>(initialVector), reinterpret_cast<const bdUByte8*>(source), reinterpret_cast<bdUByte8*>(dest), size);
    return decryptSuccess;
}

bdBool bdCryptoUtils::encrypt(const void* key, const void* initialVector, const void* source, void* dest, const bdUInt size)
{
    bdBool encryptSuccess = false;

    bdAssert((size & 7) == 0, "Source data length must be a multiple of 8 for 3DES");
    if ((size & 7) == 0)
    {
        //bdLogError("Source data length must be a multiple of 8 for 3DES");
        return false;
    }
    bdCypher3Des cypher;
    cypher.init(reinterpret_cast<const bdUByte8*>(key), 24);
    encryptSuccess = cypher.encrypt(reinterpret_cast<const bdUByte8*>(initialVector), reinterpret_cast<const bdUByte8*>(source), reinterpret_cast<bdUByte8*>(dest), size);
    return encryptSuccess;
}

bdBool bdCryptoUtils::encryptAES(const void* key, const void* initialVector, const void* source, void* dest, bdUInt size, bdUInt keySize)
{
    bdBool decryptSuccess = false;

    bdAssert((size & 0xF) == 0, "Source data length must be a multiple of 16 for AES");
    if ((size & 0xF) == 0)
    {
        //bdLogError("Source data length must be a multiple of 16 for AES");
        return false;
    }
    bdCypherAES cypher;
    bdAssert((keySize == BD_AES128_KEY_SIZE || keySize == BD_AES192_KEY_SIZE || keySize == BD_AES256_KEY_SIZE), "Cannot decryptAES with key of size[%u] (Key must be 16, 24 or 32 bytes)");
    cypher.init(reinterpret_cast<const bdUByte8*>(key), keySize);
    decryptSuccess = cypher.encrypt(reinterpret_cast<const bdUByte8*>(initialVector), reinterpret_cast<const bdUByte8*>(source), reinterpret_cast<bdUByte8*>(dest), size);
    return decryptSuccess;
}

void bdCryptoUtils::getNewEncryptionKey(bdUByte8* aesKey)
{
    bdMemset(aesKey, 0, 24);
    bdSingleton<bdTrulyRandomImpl>::getInstance()->getRandomUByte8(aesKey, 24);
}

bdUInt32 bdCryptoUtils::getNewIVSeed()
{
    return bdSingleton<bdTrulyRandomImpl>::getInstance()->getRandomUInt();
}
