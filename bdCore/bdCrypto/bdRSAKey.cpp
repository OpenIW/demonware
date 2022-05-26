// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdCore/bdCore.h"

bdRSAKey::bdRSAKey()
{
    m_status = BD_RSA_KEY_UNINITIALIZED;
    bdMemcpy(&ltc_mp, &ltm_desc, sizeof(ltc_mp));
}

bdRSAKey::~bdRSAKey()
{
    if (m_status == BD_RSA_KEY_INITIALIZED)
    {
        rsa_free(&m_key);
    }
}

bdBool bdRSAKey::init()
{
    bdInt32 error;
    prng_state prng;
    bdBool result;

    if (m_status)
    {
        bdLogWarn("rsakey", "Cannot reinitialize key.");
        return false;
    }
    if (register_prng(&yarrow_desc) == -1)
    {
        bdLogError("rsakey", "Register PRNG failed.");
        return false;
    }
    result = getStatePRNG(prng);
    if (!result)
    {
        return false;
    }
    error = rsa_make_key(&prng, find_prng("yarrow"), 128, 0x10001, &m_key);
    if (error)
    {
        bdLogError("rsakey", "Unable to create private key %s.", error_to_string(error));
        return false;
    }
    m_status = BD_RSA_KEY_INITIALIZED;
    return true;
}

bdBool bdRSAKey::getStatePRNG(prng_state& prng)
{
    bdInt32 error;
    bdUByte8 random[128];

    bdSingleton<bdTrulyRandomImpl>::getInstance()->getRandomUByte8(random, 128);
    if (register_prng(&yarrow_desc) == -1)
    {
        bdLogError("rsakey", "Register PRNG failed.");
        return false;
    }
    error = yarrow_start(&prng);
    if (error)
    {
        bdLogError("rsakey", "Start error %s.", error_to_string(error));
        return false;
    }
    error = yarrow_add_entropy(random, 128, &prng);
    if (error)
    {
        bdLogError("rsakey", "Add_entropy error %s.", error_to_string(error));
        return false;
    }
    error = yarrow_ready(&prng);
    if (error)
    {
        bdLogError("rsakey", "Ready error %s.", error_to_string(error));
        return false;
    }
    return true;

}

bdBool bdRSAKey::import(const bdNChar8* const keyBase64)
{
    bdInt keySize;
    bdNChar8 key[1024];
    bdInt error;

    keySize = bdBase64::decode(keyBase64, bdStrnlen(keyBase64, 4096), key, 1024);
    memcpy(&ltc_mp, &ltm_desc, sizeof(ltm_desc));
    error = rsa_import(reinterpret_cast<const unsigned char*>(key), keySize, &m_key);
    if (error)
    {
        bdLogError("rsakey", "rsa_import %s", error_to_string(error));
        return false;
    }
    m_status = BD_RSA_KEY_INITIALIZED;
    return true;
}

bdBool bdRSAKey::verifyHash(bdNChar8* signature, void* data, bdULong datalen)
{
    bdInt error;
    bdInt status;
    bdUInt hashSize;
    bdHashTiger192 tigerHash;
    bdUByte8 hash[24];

    memcpy(&ltc_mp, &ltm_desc, sizeof(ltc_mp));
    hashSize = 24;
    tigerHash.hash(reinterpret_cast<const bdUByte8*>(data), datalen, hash, hashSize);
    error = rsa_verify_hash(reinterpret_cast<const unsigned char*>(signature), 128, hash, hashSize, find_hash("tiger"), 0, &status, &m_key);
    if (error)
    {
        bdLogError("rsakey", "rsa_verify_hash %s", error_to_string(error));
        return false;
    }
    return true;
}

bdBool bdRSAKey::exportKey(bdUByte8* key, bdULong& keySize)
{
    bdInt error;

    if (m_status != BD_RSA_KEY_INITIALIZED)
    {
        bdLogWarn("rsakey", "Exporting uninitialized private key.");
        return false;
    }
    error = rsa_export(key, &keySize, 0, &m_key);
    if (error)
    {
        bdLogError("rsakey", "rsa_export %s", error_to_string(error));
        return false;
    }
    return true;
}

bdBool bdRSAKey::importKey(const bdUByte8* key, const bdULong keySize)
{
    bdInt error;

    memcpy(&ltc_mp, &ltm_desc, sizeof(ltc_mp));
    if (m_status == BD_RSA_KEY_INITIALIZED)
    {
        bdLogWarn("rsakey", "Importing into initialized public key.");
        return false;
    }
    error = rsa_import(key, keySize, &m_key);
    if (error)
    {
        bdLogError("rsakey", "rsa_import %s", error_to_string(error));
        return false;
    }
    m_status = BD_RSA_KEY_INITIALIZED;
    return true;
}

bdBool bdRSAKey::encrypt(const bdUByte8* dataToEncrypt, const bdULong& dataSize, bdUByte8* out, bdULong& outSize)
{
    bdInt prng_idx;
    bdInt hash_idx;
    bdInt error;
    prng_state prng;

    if (dataSize > 86)
    {
        bdLogWarn("rsakey", "Size of data to encrypt it too large.");
    }
    if (m_status == BD_RSA_KEY_UNINITIALIZED)
    {
        bdLogWarn("rsakey", "Can't encrypt uninitialized public key");
        return false;
    }
    if (register_hash(&sha1_desc) == -1)
    {
        bdLogError("rsakey", "Error registering sha1");
        return true;
    }
    hash_idx = find_hash("sha1");
    getStatePRNG(prng);
    prng_idx = find_prng("yarrow");
    error = rsa_encrypt_key(dataToEncrypt, dataSize, out, &outSize, reinterpret_cast<const unsigned char*>("DW-RSAENC"), bdStrlen("DW-RSAENC"), &prng, prng_idx, hash_idx, &m_key);
    if (error)
    {
        bdLogError("rsakey", "rsa_encrypt_key %s", error_to_string(error));
        return false;
    }
    return true;
}

bdBool bdRSAKey::decrypt(const bdUByte8* dataToDecrypt, const bdULong& dataSize, bdUByte8* out, bdULong& outSize)
{
    bdInt error;
    bdInt hash_idx;
    bdInt res;

    if (register_hash(&sha1_desc) == -1)
    {
        bdLogError("rsakey", "Error registering sha1");
        return true;
    }
    memcpy(&ltc_mp, &ltm_desc, sizeof(ltc_mp));
    hash_idx = find_hash("sha1");
    res = 0;
    error = rsa_decrypt_key(dataToDecrypt, dataSize, out, &outSize, reinterpret_cast<const unsigned char*>("DW-RSAENC"), bdStrlen("DW-RSAENC") + 1, hash_idx, &res, &m_key);
    if (error)
    {
        bdLogError("rsakey", "rsa_decrypt_key %s", error_to_string(error));
        return false;
    }
    return true;
}
