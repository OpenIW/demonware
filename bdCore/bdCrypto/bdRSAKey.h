// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdRSAKey
{
public:
    enum bdRSAKeyStatus : __int32
    {
        BD_RSA_KEY_UNINITIALIZED = 0x0,
        BD_RSA_KEY_INITIALIZED = 0x1,
    };
protected:
    bdRSAKeyStatus m_status;
    rsa_key m_key;
public:
    bdRSAKey();
    ~bdRSAKey();
    bdBool init();
    bdBool getStatePRNG(prng_state& prng);
    bdBool import(const bdNChar8* const keyBase64);
    bdBool verifyHash(bdNChar8* signature, void* data, bdULong datalen);
    bdBool exportKey(bdUByte8* key, bdULong& keySize);
    bdBool importKey(const bdUByte8* key, const bdULong keySize);
    bdBool encrypt(const bdUByte8* dataToEncrypt, const bdULong& dataSize, bdUByte8* out, bdULong& outSize);
    bdBool decrypt(const bdUByte8* dataToDecrypt, const bdULong& dataSize, bdUByte8* out, bdULong& outSize);
};
