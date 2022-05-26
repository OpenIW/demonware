// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdLobby/bdLobby.h"

const bdUInt32 BD_MAGIC_NUMBER = 4022185438;

bdAuthInfo::bdAuthInfo()
    : m_titleID(0), m_IVSeed(0)
{
    bdMemset(m_data, 0, sizeof(m_data));
    bdMemset(m_sessionKey, 0, sizeof(m_sessionKey));
}

void bdAuthService::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdAuthService::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdAuthService::bdAuthService(bdUInt titleID, const bdAddr& addr)
    : bdLobbyConnectionListener(), m_usernamesForLicenseResult(0u), m_connection(), m_authServiceAddr(), m_request(), m_authInfo(), m_authTicket(), m_errorCode(BD_NO_ERROR),
    m_sendBufSize(0), m_recvBufSize(0), m_cdKeyTimeToLiveSecs(0), m_RSAKey(), m_titleID(titleID)
{
    m_authServiceAddr = new bdCommonAddr(addr);
    m_status = BD_READY;
}

bdAuthService::~bdAuthService()
{
    m_usernamesForLicenseResult.clear();
}

bdBool bdAuthService::createAccount(const bdNChar8* userName, const bdNChar8* password, const bdNChar8* cdKey)
{
    if (m_status)
    {
        return false;
    }
    m_request = makeCreateAccount(m_titleID, userName, password, cdKey);
    startTask();
    return true;
}

bdBool bdAuthService::authorizeAccount(const bdNChar8* accountName, const bdNChar8* userKey)
{
    bdAuthUtility::getUserKey(userKey, m_userKey);
    if (m_status)
    {
        return false;
    }
    m_request = makeAuthAccountForService(m_titleID, accountName);
    startTask();
    return true;
}

bdBool bdAuthService::authorizeDedicatedHost(const bdNChar8* licenseKey)
{
    bdAuthUtility::getLicenseKey(licenseKey, m_licenseKey);
    if (m_status)
    {
        return false;
    }
    m_request = makeAuthHostForService(m_titleID, licenseKey);
    startTask();
    return true;
}

bdBool bdAuthService::getUsernamesForLicense(const bdNChar8* licenseKey)
{
    bdAuthUtility::getLicenseKey(licenseKey, m_licenseKey);
    if (m_status)
    {
        return false;
    }
    m_request = makeGetUsernamesForLicense(m_titleID, licenseKey);
    startTask();
    return true;
}

bdUByte8* bdAuthService::getSteamRequestData(const bdUByte8* steamCookieKey, bdUInt& steamCookieKeySize)
{
    createSteamCookie(m_steamCookieKey);
    bdMemset(&m_steamCookieKey[24], 0, 64);
    bdStrlcpy(reinterpret_cast<bdNChar8*>(&m_steamCookieKey[24]), reinterpret_cast<const bdNChar8*>(steamCookieKey), 64);
    steamCookieKeySize = 88;
    return m_steamCookieKey;
}

bdBool bdAuthService::authorizeSteamTicket(bdNChar8* ticket, const bdUInt ticketSize)
{
    if (m_status)
    {
        return false;
    }
    m_request = makeAuthForSteam(m_titleID, ticket, ticketSize);
    startTask();
    return true;
}

void bdAuthService::createSteamCookie(bdUByte8* cookie)
{
    createAuthCookie(cookie);
}

bdAuthService::bdStatus bdAuthService::getStatus()
{
    bdBitBufferRef message;
    bdUByte8 messageType;

    if (*m_connection && m_connection->getMessageToDispatch(messageType, message))
    {
        m_errorCode = handleReply(messageType, message);
        m_status = BD_READY;
        m_connection->close();
        m_connection = (bdLobbyConnection*)NULL;
    }
    return m_status;
}

bdLobbyErrorCode bdAuthService::getErrorCode()
{
    return m_errorCode;
}

bdAuthInfo* bdAuthService::getAuthInfo()
{
    return &m_authInfo;
}

bdAuthTicket* bdAuthService::getAuthTicket()
{
    return &m_authTicket;
}

void bdAuthService::startTask()
{
    if (m_sendBufSize)
    {
        m_connection = new bdLobbyConnection(m_authServiceAddr, m_sendBufSize, m_recvBufSize, this);
    }
    else
    {
        m_connection = new bdLobbyConnection(m_authServiceAddr, this);
    }

    if (m_connection->connect(m_authInfo))
    {
        m_status = BD_CONNECTING;
    }
    else
    {
        m_errorCode = BD_START_TASK_FAILED;
    }
}

void bdAuthService::onConnect(bdLobbyConnectionRef lobbyConnection)
{
    bdUByte8 buffer[4];
    if (*lobbyConnection && *m_request)
    {
        *buffer = 0;
        m_connection->sendRaw(buffer, sizeof(buffer));
        m_connection->send(m_request->getData(), m_request->getDataSize(), false);
        m_status = BD_PENDING_REPLY;
    }
    else if (*m_request)
    {
        m_status = BD_READY;
        m_errorCode = BD_CONNECTION_RESET;
        bdLogWarn("auth service", "Received onConnect callback for NULL connection");
    }
    else
    {
        m_status = BD_READY;
        m_errorCode = BD_START_TASK_FAILED;
        bdLogWarn("auth service", "Received onConnect callback for NULL connection");
    }
}

void bdAuthService::onDisconnect(bdLobbyConnectionRef lobbyConnection)
{
    if (m_status)
    {
        m_status = BD_READY;
        m_errorCode = BD_CONNECTION_RESET;
        bdLogInfo("auth service", "Received onDisconnect callback  for connection");
    }
}

void bdAuthService::onConnectFailed(bdLobbyConnectionRef lobbyConnection)
{
    if (m_status == BD_CONNECTING)
    {
        m_status = BD_READY;
        m_errorCode = BD_CONNECTION_RESET;
        bdLogInfo("auth service", "Received onConnectFailed callback for connection");
    }
    else
    {
        bdLogError("auth service", "Unexpected value for m_status : %u ", m_status);
    }
}

bdBitBufferRef bdAuthService::makeCreateAccount(const bdUInt titleID, const bdNChar8* userName, const bdNChar8* password, const bdNChar8* cdKey)
{
    bdUByte8 licenseBuffer[8];
    bdUByte8 credentialsBuffer[96];
    bdUByte8 encCredentials[96];
    bdNChar8 licenseKey[24];
    bdUByte8 iv[24];

    bdBitBufferRef buffer(new bdBitBuffer(8, true));
    buffer->setTypeCheck(false);
    buffer->writeBool(true);
    buffer->setTypeCheck(true);

    // Calculate our initialVector
    bdUInt ivseed = bdCryptoUtils::getNewIVSeed();
    bdCryptoUtils::calculateInitialVector(ivseed, iv);

    // Write our IV seed & the titleID to the buffer
    buffer->writeDataType(BD_BB_UNSIGNED_INTEGER32_TYPE);
    buffer->writeBits(&ivseed, 32);
    buffer->writeDataType(BD_BB_UNSIGNED_INTEGER32_TYPE);
    buffer->writeBits(&titleID, 32);

    // Serialize the licenseID
    bdAuthCreateAccountPlainText license;
    license.m_licenseID = bdAuthUtility::getLicenseID(cdKey);
    license.serialize(licenseBuffer, sizeof(licenseBuffer));

    // Serialize credentials
    bdAuthCreateAccountCypherText credentials;
    credentials.m_magicNumber = BD_MAGIC_NUMBER;
    bdMemset(credentials.m_username, 0, sizeof(credentials.m_username));
    bdStrlcpy(credentials.m_username, userName, sizeof(credentials.m_username));
    bdAuthUtility::getUserKey(password, credentials.m_userKey);
    credentials.serialize(credentialsBuffer, sizeof(credentialsBuffer));

    // Get licenseKey from the cdKey and use it to encrypt our credentials
    bdAuthUtility::getLicenseKey(cdKey, licenseKey);
    bdCryptoUtils::encrypt(licenseKey, iv, credentialsBuffer, encCredentials, sizeof(credentialsBuffer));

    // Write our license and encrypted credentials to the buffer
    buffer->writeBits(licenseBuffer, 64);
    buffer->writeBits(encCredentials, sizeof(encCredentials) * CHAR_BIT);
    return &buffer;
}

bdBitBufferRef bdAuthService::makeGetUsernamesForLicense(const bdUInt titleID, const bdNChar8* licenseKey)
{
    bdUByte8 iv[24];
    bdUByte8 licenseBuffer[8];

    bdBitBufferRef buffer(new bdBitBuffer(8, true));
    buffer->setTypeCheck(false);
    buffer->writeBool(true);
    buffer->setTypeCheck(true);
    bdUInt ivseed = bdCryptoUtils::getNewIVSeed();
    bdCryptoUtils::calculateInitialVector(ivseed, iv);
    buffer->writeDataType(BD_BB_UNSIGNED_INTEGER32_TYPE);
    buffer->writeBits(&ivseed, 32);
    buffer->writeDataType(BD_BB_UNSIGNED_INTEGER32_TYPE);
    buffer->writeBits(&titleID, 32);

    bdAuthCreateAccountPlainText license;
    license.m_licenseID = bdAuthUtility::getLicenseID(licenseKey);
    license.serialize(licenseBuffer, sizeof(licenseBuffer));
    buffer->writeBits(licenseBuffer, sizeof(licenseBuffer) * CHAR_BIT);
    return &buffer;
}

bdBitBufferRef bdAuthService::makeAuthAccountForService(const bdUInt titleID, const bdNChar8* accountName)
{
    bdUByte8 iv[24];
    bdUByte8 licenseBuffer[8];

    bdBitBufferRef buffer(new bdBitBuffer(8, true));
    buffer->setTypeCheck(false);
    buffer->writeBool(true);
    buffer->setTypeCheck(true);
    bdUInt ivseed = bdCryptoUtils::getNewIVSeed();
    bdCryptoUtils::calculateInitialVector(ivseed, iv);
    buffer->writeDataType(BD_BB_UNSIGNED_INTEGER32_TYPE);
    buffer->writeBits(&ivseed, 32);
    buffer->writeDataType(BD_BB_UNSIGNED_INTEGER32_TYPE);
    buffer->writeBits(&titleID, 32);
    
    bdAuthCreateAccountPlainText license;
    license.m_licenseID = bdAuthUtility::getLicenseID(accountName);
    license.serialize(licenseBuffer, sizeof(licenseBuffer));
    buffer->writeBits(licenseBuffer, sizeof(licenseBuffer) * CHAR_BIT);
    return &buffer;
}

bdBitBufferRef bdAuthService::makeAuthHostForService(const bdUInt titleID, const bdNChar8* licenseKey)
{
    bdUByte8 iv[24];
    bdUByte8 licenseBuffer[8];

    bdBitBufferRef buffer(new bdBitBuffer(8, true));
    buffer->setTypeCheck(false);
    buffer->writeBool(true);
    buffer->setTypeCheck(true);

    bdUInt ivseed = bdCryptoUtils::getNewIVSeed();
    bdCryptoUtils::calculateInitialVector(ivseed, iv);
    buffer->writeDataType(BD_BB_UNSIGNED_INTEGER32_TYPE);
    buffer->writeBits(&ivseed, 32);
    buffer->writeDataType(BD_BB_UNSIGNED_INTEGER32_TYPE);
    buffer->writeBits(&titleID, 32);

    bdAuthCreateAccountPlainText license;
    license.m_licenseID = bdAuthUtility::getLicenseID(licenseKey);
    license.serialize(licenseBuffer, sizeof(licenseBuffer));
    buffer->writeBits(licenseBuffer, sizeof(licenseBuffer) * CHAR_BIT);
    return &buffer;
}

bdLobbyErrorCode bdAuthService::handleReply(bdUByte8 replyType, bdBitBufferRef reply)
{
    bdUInt errorCode;

    if (!reply->readUInt32(errorCode))
    {
        bdLogError("auth service", "Could not read error code from authentication server response.");
        return BD_NO_ERROR;
    }
    if (errorCode != BD_AUTH_NO_ERROR)
    {
        bdLogInfo("auth service", "Task returned with error code %u\n", errorCode);
        return static_cast<bdLobbyErrorCode>(errorCode);
    }

    bdUByte8 iv[24];
    bdNChar8 username[64];
    bdUByte8 deCypherText[128];
    bdUByte8 cypherText[128];
    bdNChar8 cdkeyObfus[86];
    bdUByte8 encCDKey[128];
    bdUInt ivseed;
    bdUInt resultCount;
    bdBool ok;

    switch (replyType)
    {

    case 11:
        ivseed = 0;
        reply->readUInt32(ivseed);
        bdCryptoUtils::calculateInitialVector(ivseed, iv);
        if (!reply->readBits(cypherText, sizeof(cypherText) * CHAR_BIT))
        {
            bdLogWarn("auth service", "Failed to read user ticket.\n");
            break;
        }
        bdCryptoUtils::decrypt(m_userKey, iv, cypherText, deCypherText, sizeof(deCypherText));
        m_authTicket.deserialize(deCypherText);
        if (m_authTicket.m_magicNumber == BD_MAGIC_NUMBER)
        {
            bdMemcpy(m_authInfo.m_sessionKey, m_authTicket.m_sessionKey, sizeof(m_authInfo.m_sessionKey));
            reply->readBits(m_authInfo.m_data, sizeof(m_authInfo.m_data) * CHAR_BIT);
            m_authInfo.m_titleID = m_authTicket.m_titleID;
            m_authInfo.m_IVSeed = ivseed;
            break;
        }
        return BD_AUTH_INCORRECT_PASSWORD;

    case 13:
        ivseed = 0;
        reply->readUInt32(ivseed);
        bdCryptoUtils::calculateInitialVector(ivseed, iv);
        if (!reply->readBits(cypherText, sizeof(cypherText) * CHAR_BIT))
        {
            bdLogWarn("auth service", "Failed to read user ticket.\n");
            break;
        }
        bdCryptoUtils::decrypt(m_licenseKey, iv, cypherText, deCypherText, sizeof(deCypherText));
        m_authTicket.deserialize(deCypherText);
        if (m_authTicket.m_magicNumber == BD_MAGIC_NUMBER)
        {
            bdMemcpy(m_authInfo.m_sessionKey, m_authTicket.m_sessionKey, sizeof(m_authInfo.m_sessionKey));
            reply->readBits(m_authInfo.m_data, sizeof(m_authInfo.m_data) * CHAR_BIT);
            m_authInfo.m_titleID = m_authTicket.m_titleID;
            m_authInfo.m_IVSeed = ivseed;
            break;
        }
        return BD_AUTH_INCORRECT_LICENSE_CODE;

    case 15:
        ivseed = 0;
        reply->readUInt32(ivseed);
        bdCryptoUtils::calculateInitialVector(ivseed, iv);
        if (reply->readBits(cypherText, sizeof(cypherText) * CHAR_BIT))
        {
            bdCryptoUtils::decrypt(m_userKey, iv, cypherText, m_authInfo.m_data, sizeof(m_authInfo.m_data));
            m_authInfo.m_titleID = 0;
            m_authInfo.m_IVSeed = ivseed;
        }
        else
        {
            bdLogWarn("auth service", "Failed to read user ticket.\n");
        }
        break;

    case 17:
        ivseed = 0;
        reply->readUInt32(ivseed);
        bdCryptoUtils::calculateInitialVector(ivseed, iv);
        if (!reply->readBits(cypherText, sizeof(cypherText) * CHAR_BIT))
        {
            bdLogWarn("auth service", "Failed to read user ticket.\n");
            break;
        }
        m_authTicket.deserialize(cypherText);
        if (m_authTicket.m_magicNumber == BD_MAGIC_NUMBER)
        {
            bdMemcpy(m_authInfo.m_sessionKey, m_authTicket.m_sessionKey, sizeof(m_authInfo.m_sessionKey));
            reply->readBits(m_authInfo.m_data, sizeof(m_authInfo.m_data) * CHAR_BIT);
            m_authInfo.m_titleID = m_authTicket.m_titleID;
            m_authInfo.m_IVSeed = ivseed;
            break;
        }
        return  BD_AUTH_UNKNOWN_ERROR;

    case 21:
        resultCount = 0;
        m_usernamesForLicenseResult.clear();
        ok = reply->readUInt32(resultCount);
        if (!ok)
        {
            bdLogWarn("auth service", "Failed to read result count from reply");
            errorCode = BD_AUTH_UNKNOWN_ERROR;
            break;
        }
        m_usernamesForLicenseResult.ensureCapacity(resultCount);
        for (bdUInt i = 0; i < resultCount && ok; ++i)
        {
            ok = reply->readBits(username, sizeof(username) * CHAR_BIT);
            m_usernamesForLicenseResult.pushBack(username);
        }
        if (!ok)
        {
            bdLogWarn("auth service", "Failed to read %u usernames from reply", resultCount);
            errorCode = BD_AUTH_UNKNOWN_ERROR;
            break;
        }
        break;

    case 25:
        ivseed = 0;
        reply->readUInt32(ivseed);
        bdCryptoUtils::calculateInitialVector(ivseed, iv);
        if (!reply->readBits(cypherText, sizeof(cypherText) * CHAR_BIT))
        {
            bdLogWarn("auth service", "Failed to read user ticket.\n");
            break;
        }
        if (!reply->readBits(m_authInfo.m_data, sizeof(m_authInfo.m_data) * CHAR_BIT))
        {
            bdLogWarn("auth service", "Failed to read LSG ticket.\n");
            break;
        }
        bdMemset(m_cdKey, 0, sizeof(m_cdKey));
        if (!reply->readBits(m_cdKey, sizeof(m_cdKey) * CHAR_BIT))
        {
            bdLogWarn("auth service", "Failed to read LSG ticket.\n");
            break;
        }
        strcpy(cdkeyObfus, "43FCB2ACF2D72593DD7CD1C69E0F03C07229F4C83166F7B05BA0C5FE3AA3A2D93EK2495783KDKN92939DK");
        for (bdUInt i = 0; i < 86; ++i)
        {
            m_cdKey[i] ^= cdkeyObfus[i];
        }
        m_cdKeyTimeToLiveSecs = 0;
        if (!reply->readInt32(m_cdKeyTimeToLiveSecs))
        {
            bdLogWarn("auth service", "Failed to read LSG ticket.\n");
            break;
        }
        bdAuthUtility::getLicenseKey(m_cdKey, m_licenseKey);
        bdCryptoUtils::decrypt(m_licenseKey, iv, cypherText, deCypherText, sizeof(deCypherText));
        m_authTicket.deserialize(deCypherText);
        if (m_authTicket.m_magicNumber == BD_MAGIC_NUMBER)
        {
            bdMemcpy(m_authInfo.m_sessionKey, m_authTicket.m_sessionKey, sizeof(m_authInfo.m_sessionKey));
            m_authInfo.m_titleID = m_authTicket.m_titleID;
            m_authInfo.m_IVSeed = ivseed;
            break;
        }
        return BD_AUTH_INCORRECT_LICENSE_CODE;

    case 27:
    {
        ivseed = 0;
        reply->readUInt32(ivseed);
        bdCryptoUtils::calculateInitialVector(ivseed, iv);
        if (!reply->readBits(cypherText, sizeof(cypherText) * CHAR_BIT))
        {
            bdLogWarn("auth service", "Failed to read user ticket.\n");
            break;
        }
        if (!reply->readBits(m_authInfo.m_data, sizeof(m_authInfo.m_data) * CHAR_BIT))
        {
            bdLogWarn("auth service", "Failed to read LSG ticket.\n");
            break;
        }
        bdULong encCDKeySize = 128;
        if (!reply->readBits(encCDKey, sizeof(encCDKey) * CHAR_BIT))
        {
            bdLogWarn("auth service", "Failed to read Encrypted cd key.\n");
            break;
        }
        bdULong outSize = 86;
        bdMemset(m_cdKey, 0, sizeof(m_cdKey));
        m_RSAKey.decrypt(encCDKey, encCDKeySize, reinterpret_cast<bdUByte8*>(m_cdKey), outSize);
        m_cdKeyTimeToLiveSecs = 0;
        if (!reply->readInt32(m_cdKeyTimeToLiveSecs))
        {
            bdLogWarn("auth service", "Failed to read LSG ticket.\n");
            break;
        }
        bdAuthUtility::getLicenseKey(m_cdKey, m_licenseKey);
        bdCryptoUtils::decrypt(m_licenseKey, iv, cypherText, deCypherText, sizeof(deCypherText));
        m_authTicket.deserialize(deCypherText);
        if (m_authTicket.m_magicNumber == BD_MAGIC_NUMBER)
        {
            bdMemcpy(m_authInfo.m_sessionKey, m_authTicket.m_sessionKey, sizeof(m_authInfo.m_sessionKey));
            m_authInfo.m_titleID = m_authTicket.m_titleID;
            m_authInfo.m_IVSeed = ivseed;
            break;
        }
        return BD_AUTH_INCORRECT_LICENSE_CODE;
    }
    
    case 29:
        if (!handleSteamReply(&reply))
        {
            return BD_AUTH_UNKNOWN_ERROR;
        }
        break;

    default:
        break;

    }
    bdLogInfo("auth service", "Task completed sucessfully\n");
    return static_cast<bdLobbyErrorCode>(errorCode);
}

void bdAuthService::createAuthCookie(bdUByte8* cookie)
{
    bdUByte8 c;

    bdMemset(cookie, 0, 24);
    bdSingleton<bdTrulyRandomImpl>::getInstance()->getRandomUByte8(cookie, 24);
    for (bdUInt i = 0; i < 24; ++i)
    {
        c = cookie[i] <= 0 ? -cookie[i] : cookie[i] % 62;
        if (c >= 0x1Au)
        {
            if (c < 0x34u)
            {
                cookie[i] = c + 39;
            }
        }
        else
        {
            cookie[i] = c + 97;
        }
        if (c >= 0x34u && c < 0x3Eu)
        {
            cookie[i] = c - 4;
        }
    }
}

bdBitBufferRef bdAuthService::makeAuthForSteam(const bdUInt titleID, const bdNChar8* ticket, const bdUInt ticketSize)
{
    bdUByte8 iv[24];

    bdBitBufferRef buffer(new bdBitBuffer(8, true));
    buffer->setTypeCheck(false);
    buffer->writeBool(true);
    buffer->setTypeCheck(true);

    bdUInt ivseed = bdCryptoUtils::getNewIVSeed();
    bdCryptoUtils::calculateInitialVector(ivseed, iv);
    buffer->writeDataType(BD_BB_UNSIGNED_INTEGER32_TYPE);
    buffer->writeBits(&ivseed, 32);
    buffer->writeDataType(BD_BB_UNSIGNED_INTEGER32_TYPE);
    buffer->writeBits(&titleID, 32);
    buffer->writeDataType(BD_BB_UNSIGNED_INTEGER32_TYPE);
    buffer->writeBits(&ticketSize, 32);
    buffer->writeBits(ticket, CHAR_BIT * ticketSize);
    return &buffer;
}

bdBool bdAuthService::handleSteamReply(bdBitBufferRef buffer)
{
    bdUByte8 iv[24];
    bdUByte8 recvdAuthTicket[128];

    bdUInt ivseed;
    bdBool ok = buffer->readUInt32(ivseed);
    bdCryptoUtils::calculateInitialVector(ivseed, iv);
    ok = ok == buffer->readBits(recvdAuthTicket, sizeof(recvdAuthTicket) * CHAR_BIT);
    bdCryptoUtils::decrypt(m_steamCookieKey, iv, recvdAuthTicket, recvdAuthTicket, sizeof(recvdAuthTicket));

    bdAuthTicket authTicket;
    if (ok)
    {
        authTicket.deserialize(recvdAuthTicket);
    }
    ok = ok == buffer->readBits(m_authInfo.m_data, sizeof(m_authInfo.m_data) * CHAR_BIT);
    if (ok && authTicket.m_magicNumber == BD_MAGIC_NUMBER)
    {
        bdMemcpy(m_authInfo.m_sessionKey, authTicket.m_sessionKey, sizeof(m_authInfo.m_sessionKey));
        m_authInfo.m_titleID = authTicket.m_titleID;
        m_authInfo.m_IVSeed = ivseed;
    }
    return ok;
}
