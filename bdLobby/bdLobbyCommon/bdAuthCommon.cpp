// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

bdUInt64 bdAuthUtility::getLicenseID(const bdNChar8* licenseCode)
{
    bdUByte8 licenseHash[24];

    if (!licenseCode)
    {
        return 0;
    }
    bdHashTiger192 tigerHash;
    bdUInt hashSize = 24;
    bdUInt licenseCodeLen = bdStrlen(licenseCode);
    bdBool success = tigerHash.hash(reinterpret_cast<const bdUByte8*>(licenseCode), licenseCodeLen, licenseHash, &hashSize);
    bdAssert(success, "Hash function failed.");
    bdUInt64 licenseID = 0;
    bdUInt offset = 0;
    bdBytePacker::removeBasicType<bdUInt64>(licenseHash, hashSize, 0, &offset, &licenseID);
    return licenseID;
}

unsigned char BD_MAGIC_LICENSE_KEY[] =
{
    0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD,
    0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

void bdAuthUtility::getLicenseKey(const bdNChar8* licenseCode, bdByte8* licenseKey)
{
    bdUByte8 licenseHash[24];

    if (!licenseCode)
    {
        bdMemcpy(licenseKey, BD_MAGIC_LICENSE_KEY, sizeof(BD_MAGIC_LICENSE_KEY));
        return;
    }
    bdHashTiger192 tigerHash;
    bdUInt hashSize = 24;
    bdUInt licenseCodeLen = bdStrlen(licenseCode);
    bdBool success = tigerHash.hash(reinterpret_cast<const bdUByte8*>(licenseCode), licenseCodeLen, licenseHash, &hashSize);
    bdAssert(success, "Hash function failed.");
    bdMemcpy(licenseKey, licenseHash, sizeof(licenseKey));
}

void bdAuthUtility::getUserKey(const bdNChar8* password, bdByte8* userKey)
{
    bdUByte8 passwordHash[24];
    bdHashTiger192 tigerHash;
    bdUInt hashSize = 24;
    bdUInt passwordLen = bdStrlen(password);
    bdBool success = tigerHash.hash(reinterpret_cast<const bdUByte8*>(password), passwordLen, passwordHash, &hashSize);
    bdAssert(success, "Hash function failed.");
    bdMemcpy(userKey, passwordHash, sizeof(passwordHash));
}

bdUInt64 bdAuthUtility::getUserID(const bdNChar8* username)
{
    bdUByte8 hashBuffer[24];
    bdNChar8 lwrUsernameBuffer[65];

    bdUInt64 userID = 0;
    bdMemset(lwrUsernameBuffer, 0, sizeof(lwrUsernameBuffer));
    bdStrlcpy(lwrUsernameBuffer, username, sizeof(lwrUsernameBuffer));
    bdUInt hashBufferSize = 24;
    bdStrlwr(lwrUsernameBuffer);
    bdMemset(hashBuffer, 0, sizeof(hashBuffer));
    bdHashTiger192 tigerHash;
    bdUInt lwrUsernameLength = bdStrnlen(lwrUsernameBuffer, sizeof(lwrUsernameBuffer));
    bdBool success = tigerHash.hash(reinterpret_cast<const bdUByte8*>(lwrUsernameBuffer), lwrUsernameLength, hashBuffer, &hashBufferSize);
    if (!success)
    {
        bdLogError("bdCore/bdAuthUtility", "Hash failed.");
        return userID;
    }
    bdUInt offset = 0;
    bdBytePacker::removeBasicType<bdUInt64>(hashBuffer, hashBufferSize, 0, &offset, &userID);
    return userID;
}

void bdAuthCreateAccountPlainText::serialize(void* buffer, const bdUInt bufferSize)
{
    bdUInt offset = 0;
    bdMemset(buffer, 0, bufferSize);
    bdBool status = bdBytePacker::appendBasicType<bdUInt64>(buffer, bufferSize, 0, &offset, &m_licenseID);
    bdAssert(status, "Failed to serialize.");
}

void bdAuthCreateAccountPlainText::deserialize(const void* buffer, const bdUInt bufferSize)
{
    bdUInt offset = 0;
    bdBool status = bdBytePacker::removeBasicType<bdUInt64>(buffer, bufferSize, 0, &offset, &m_licenseID);
    bdAssert(status, "Failed to deserialize.");
}

void bdAuthCreateAccountCypherText::serialize(void* buffer, const bdUInt bufferSize)
{
    bdUInt offset = 0;
    bdMemset(buffer, 0, sizeof(bufferSize));
    bdBool status = bdBytePacker::appendBasicType<bdUInt>(buffer, bufferSize, 0, &offset, &m_magicNumber);
    status = status == bdBytePacker::appendBuffer(reinterpret_cast<bdUByte8*>(buffer), bufferSize, offset, &offset, m_username, sizeof(m_username));
    status = status == bdBytePacker::appendBuffer(reinterpret_cast<bdUByte8*>(buffer), bufferSize, offset, &offset, m_userKey, sizeof(m_userKey));
    bdAssert(status, "Failed to serialize.");
}

void bdAuthCreateAccountCypherText::deserialize(const void* buffer, const bdUInt bufferSize)
{
    bdUInt offset = 0;
    bdBool status = bdBytePacker::removeBasicType<bdUInt>(buffer, bufferSize, 0, &offset, &m_magicNumber);
    status = status == bdBytePacker::removeBuffer(reinterpret_cast<const bdUByte8*>(buffer), bufferSize, offset, &offset, m_username, sizeof(m_username));
    status = status == bdBytePacker::removeBuffer(reinterpret_cast<const bdUByte8*>(buffer), bufferSize, offset, &offset, m_userKey, sizeof(m_userKey));
    bdAssert(status, "Failed to deserialize.");
}
