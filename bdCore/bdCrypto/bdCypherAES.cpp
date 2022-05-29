// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdCore/bdCore.h"

bdCypherAES::bdCypherAES() : bdCypher(16)
{
    if (register_cipher(&aes_desc) == -1)
    {
        //bdLogError("Error registering AES cipher: [%s]", error_to_string(1);
    }
}

bdCypherAES::~bdCypherAES()
{
}

void* bdCypherAES::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

void* bdCypherAES::operator new(bdUWord __needed, void* p)
{
    return p;
}

void bdCypherAES::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

bdBool bdCypherAES::init(const bdUByte8* key, bdUInt keySize)
{
    bdUByte8 iv[16];
    bdUInt index;
    bdInt error;

    if (keySize != 16 && keySize != 24 && keySize != 32)
    {
        //bdLogError("Cannot init AES cypher with key of size [%u], Key must be 16, 24 or 32 bytes", keySize);
        return false;
    }
    index = find_cipher_id(aes_desc.ID);
    if (index == -1)
    {
        //bdLogError("Error retrieving AES cipher");
        return false;
    }
    bdMemset(iv, 0, 16);
    error = cbc_start(index, iv, key, keySize, 0, &m_cbc);
    if (error)
    {
        //bdLogError("Error starting cipher: [%s]", error_to_string(error));
        return false;
    }
    m_keySize = keySize;
    return true;
}

bdBool bdCypherAES::decrypt(const bdUByte8* iv, const bdUByte8* cypherText, bdUByte8* plainText, bdUInt size)
{
    bdInt error;

    error = cbc_setiv(iv, 16, &m_cbc);
    if (error)
    {
        //bdLogError("Failed to set IV seed: [%s]", error_to_string(error));
        return false;
    }
    error = cbc_decrypt(cypherText, plainText, size, &this->m_cbc);
    if (error)
    {
        //bdLogError("Error decrypting: [%s]", error_to_string(error));
        return false;
    }
    return true;
}

bdBool bdCypherAES::encrypt(const bdUByte8* iv, const bdUByte8* plainText, bdUByte8* cypherText, bdUInt size)
{
    bdInt error;

    error = cbc_setiv(iv, 16, &m_cbc);
    if (error)
    {
        //bdLogError("Failed to set IV seed: [%s]", error_to_string(error));
        return false;
    }
    error = cbc_encrypt(plainText, cypherText, size, &this->m_cbc);
    if (error)
    {
        //bdLogError("Error encrypting: [%s]", error_to_string(error));
        return false;
    }
    return true;
}
