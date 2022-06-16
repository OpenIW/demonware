// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdCore/bdCore.h"

bdCypher3Des::bdCypher3Des() : bdCypher(8u)
{
    bdInt error;

    if (register_cipher(&des3_desc) == -1)
    {
        bdLogError("bdCypher3Des", "Error registering 3DES cipher: [%s]", error_to_string(1));
    }
}

bdCypher3Des::~bdCypher3Des()
{
}

void* bdCypher3Des::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

void* bdCypher3Des::operator new(bdUWord __needed, void* p)
{
    return p;
}

void bdCypher3Des::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

bdBool bdCypher3Des::init(const bdUByte8* key, bdUInt keySize)
{
    bdUByte8 iv[8];
    bdInt index;
    bdInt error;

    if (keySize != 24)
    {
        bdLogError("bdCypher3Des", "Cannot init 3DES cypher with key of size [%u], Key must 24 bytes", keySize);
        return false;
    }
    index = find_cipher_id(des3_desc.ID);
    if (index == -1)
    {
        bdLogError("bdCypher3Des", "Error retrieving 3DES cipher");
        return false;
    }
    bdMemset(iv, 0, 8);
    error = cbc_start(index, iv, key, 24, 0, &m_cbc);
    if (error)
    {
        bdLogError("bdCypher3Des", "Error starting cipher: [%s]", error_to_string(error));
        return false;
    }
    return true;
}

bdBool bdCypher3Des::decrypt(const bdUByte8* iv, const bdUByte8* cypherText, bdUByte8* plainText, bdUInt size)
{
    bdInt error;

    error = cbc_setiv(iv, 8uLL, &m_cbc);
    if (error)
    {
        bdLogError("bdCypher3DES", "Failed to set IV seed: [%s]", error_to_string(error));
        return false;
    }
    if (cbc_decrypt(cypherText, plainText, size, &m_cbc))
    {
        bdLogError("bdCypher3DES", "Error decrypting 3DES data");
        return false;
    }
    return true;
}

bdBool bdCypher3Des::encrypt(const bdUByte8* iv, const bdUByte8* plainText, bdUByte8* cypherText, bdUInt size)
{
    bdInt error;

    error = cbc_setiv(iv, 8uLL, &m_cbc);
    if (error)
    {
        bdLogError("bdCypher3DES", "Failed to set IV seed: [%s]", error_to_string(error));
        return false;
    }
    error = cbc_encrypt(plainText, cypherText, size, &m_cbc);
    if (error)
    {
        bdLogError("bdCypher3DES", "Error encrypting: [%s]", error_to_string(error));
        return false;
    }
    return true;
}
