// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdCore/bdCore.h"

bdECCKey::bdECCKey()
    : m_status(BD_ECC_KEY_UNINITIALIZED)
{
    m_status = BD_ECC_KEY_UNINITIALIZED;
    bdMemcpy(&ltc_mp, &ltm_desc, sizeof(ltc_math_descriptor));
}

bdECCKey::~bdECCKey()
{
    if (m_status == BD_ECC_KEY_INITIALIZED)
    {
        ecc_free(&m_key);
    }
    m_status = BD_ECC_KEY_UNINITIALIZED;
}

bdBool bdECCKey::init()
{
    bdUByte8 random[128];
    bdInt error;
    prng_state prng;

    if (m_status == BD_ECC_KEY_INITIALIZED)
    {
        bdLogWarn("bdECCKey", "Cannot reinitialize key");
        return false;
    }
    if (register_prng(&yarrow_desc) == -1)
    {
        bdLogError("bdECCKey", "Register PRNG failed");
        return false;
    }
    bdSingleton<bdTrulyRandomImpl>::getInstance()->getRandomUByte8(random, 128);
    error = yarrow_start(&prng);
    if (error)
    {
        bdLogError("bdECCKey", "Start error %s.", error_to_string(error));
        return false;
    }
    error = yarrow_add_entropy(random, 128, &prng);
    if (error)
    {
        bdLogError("bdECCKey", "Add_entropy error %s.", error_to_string(error));
        return false;
    }
    error = yarrow_ready(&prng);
    if (error)
    {
        bdLogError("bdECCKey", "Ready error %s.", error_to_string(error));
        return false;
    }
    error = ecc_make_key(&prng, find_prng("yarrow"), 28, &m_key);
    if (error)
    {
        bdLogError("bdECCKey", "Unable to create private key: [%s]", error_to_string(error));
        return false;
    }
    m_status = BD_ECC_KEY_INITIALIZED;
    return true;
}

bdBool bdECCKey::exportKey(bdUByte8* const pubKey, bdUInt& keyLen)
{
    bdInt error;
    bdULong tmpKeylen;

    if (m_status != BD_ECC_KEY_INITIALIZED)
    {
        bdLogWarn("bdECCKey", "Can only export initialized private key");
        return false;
    }
    tmpKeylen = keyLen;
    error = ecc_export(pubKey, &tmpKeylen, 0, &m_key);
    if (error)
    {
        bdLogError("bdECCKey", "Unable to export public key: [%s]", error_to_string(error));
        return false;
    }
    keyLen = tmpKeylen;
    if (keyLen < 100)
    {
        bdMemset(&pubKey[keyLen], 0, 100 - keyLen);
    }
    keyLen = 100;
    return true;
}

bdBool bdECCKey::generateSharedSecret(const bdUByte8* const publicKey, const bdUInt publicKeyLen, bdUByte8* secretBuffer, const bdUInt secretBufferLen)
{
    bdInt error;
    bdInt importError;
    bdULong tmpLen;
    bdECCKey importedKey;

    importError = ecc_import(publicKey, publicKeyLen, &importedKey.m_key);
    if (importError)
    {
        bdLogError("bdECCKey", "Unable to import public key: [%s]", error_to_string(importError));
        return false;
    }
    if (m_status != BD_ECC_KEY_INITIALIZED)
    {
        bdLogWarn("bdECCKey", "Cannot generate secret with uninitialized key");
        ecc_free(&importedKey.m_key);
        return true;
    }
    tmpLen = secretBufferLen;
    error = ecc_shared_secret(&m_key, &importedKey.m_key, secretBuffer, &tmpLen);
    if (error)
    {
        bdLogError("bdECCKey", "Unable to generate secret key: [%s]", error_to_string(importError));
        return false;
    }
    if (tmpLen < secretBufferLen)
    {
        bdMemset(&secretBuffer[tmpLen], 0, secretBufferLen - tmpLen);
    }
    return true;
}

bdECCKey::bdECCKeyStatus bdECCKey::getStatus()
{
    return m_status;
}
