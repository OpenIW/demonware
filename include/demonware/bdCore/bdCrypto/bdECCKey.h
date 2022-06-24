// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdECCKey
{
public:
    enum bdECCKeyStatus : bdInt
    {
        BD_ECC_KEY_UNINITIALIZED = 0x0,
        BD_ECC_KEY_INITIALIZED = 0x1,
    };
    bdECCKeyStatus m_status;
    ecc_key m_key;

    bdECCKey();
    ~bdECCKey();
    bdBool init();
    bdBool exportKey(bdUByte8* const pubKey, bdUInt& keyLen);
    bdBool generateSharedSecret(const bdUByte8* const publicKey, const bdUInt publicKeyLen, bdUByte8* secretBuffer, const bdUInt secretBufferLen);
    bdECCKeyStatus getStatus();
};
