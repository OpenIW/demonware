// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdCryptoUtils
{
public:
    static void calculateInitialVector(const bdUInt32 seed, bdUByte8* iv);
    static bdBool decrypt(const void* key, const void* initialVector, const void* source, void* dest, const bdUInt size);
    static bdBool decryptAES(const void* key, const void* initialVector, const void* source, void* dest, bdUInt size, bdUInt keySize);
    static bdBool encrypt(const void* key, const void* initialVector, const void* source, void* dest, const bdUInt size);
    bdBool encryptAES(const void* key, const void* initialVector, const void* source, void* dest, bdUInt size, bdUInt keySize);
    static void getNewEncryptionKey(bdUByte8* aesKey);
    static bdUInt32 getNewIVSeed();
};