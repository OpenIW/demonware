// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdAuthInfo
{
public:
    bdUInt m_titleID;
    bdUInt m_IVSeed;
    bdNChar8 m_data[128];
    bdUByte8 m_sessionKey[24];

    bdAuthInfo();
};

class bdAuthService : public bdLobbyConnectionListener
{
public:
    enum bdStatus
    {
        BD_READY = 0,
        BD_CONNECTING = 1,
        BD_PENDING_REPLY = 2
    };
    bdFastArray<bdNChar8[64]> m_usernamesForLicenseResult;
protected:
    bdLobbyConnectionRef m_connection;
    bdUInt m_titleID;
    bdCommonAddrRef m_authServiceAddr;
    bdAuthService::bdStatus m_status;
    bdBitBufferRef m_request;
    bdAuthInfo m_authInfo;
    bdAuthTicket m_authTicket;
    bdUByte8 m_steamCookieKey[88];
    bdNChar8 m_userKey[24];
    bdNChar8 m_licenseKey[24];
    bdLobbyErrorCode m_errorCode;
    bdUInt m_sendBufSize;
    bdUInt m_recvBufSize;
    bdNChar8 m_cdKey[86];
    bdInt m_cdKeyTimeToLiveSecs;
    bdRSAKey m_RSAKey;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdAuthService(bdUInt titleID, const bdAddr& addr);
    ~bdAuthService();
    bdBool createAccount(const bdNChar8* userName, const bdNChar8* password, const bdNChar8* cdKey);
    bdBool changePassword(char*, char*, char*);
    bdBool resetAccount(char*, char*, char*);
    bdBool deleteAccount(char*, char*);
    bdBool migrateAccount(char*, const unsigned int, char*);
    bdBool authorizeAccount(const bdNChar8* accountName, const bdNChar8* userKey);
    bdBool authorizeAnonymous();
    bdBool authorizeDedicatedHost(const bdNChar8* licenseKey);
    bdBool authorizeDedicatedServer();
    bdBool authorizeDedicatedServerRSA();
    bdBool getUsernamesForLicense(const bdNChar8* licenseCode);
    bdUByte8* getSteamRequestData(const bdNChar8* steamCookieKey, bdUInt& steamCookieKeySize);
    bdBool authorizeSteamTicket(bdNChar8* ticket, const bdUInt ticketSize);
    static void createSteamCookie(bdUByte8* cookie);
    void setSteamCookie(char*);
    bdAuthService::bdStatus getStatus();
    bdLobbyErrorCode getErrorCode();
    bdAuthInfo* getAuthInfo();
    bdAuthTicket* getAuthTicket();
protected:
    void startTask();
    void onConnect(bdLobbyConnectionRef lobbyConnection);
    void onDisconnect(bdLobbyConnectionRef lobbyConnection);
    void onConnectFailed(bdLobbyConnectionRef lobbyConnection);
    bdBitBufferRef makeCreateAccount(const bdUInt titleID, const bdNChar8* userName, const bdNChar8* password, const bdNChar8* cdKey);
    bdBitBufferRef makeGetUsernamesForLicense(const bdUInt titleID, const bdNChar8* licenseKey);
    bdBitBufferRef makeChangeUserKey(const unsigned int, char*, char*, char*);
    bdBitBufferRef makeResetAccount(const unsigned int, char*, char*, char*);
    bdBitBufferRef makeDeleteAccount(const unsigned int, char*, char*);
    bdBitBufferRef makeMigrateAccount(const unsigned int, char*, const unsigned int, char*);
    bdBitBufferRef makeAuthAccountForService(const bdUInt titleID, const bdNChar8* accountName);
    bdBitBufferRef makeAuthAnonymousForService(const unsigned int);
    bdBitBufferRef makeAuthHostForService(const bdUInt titleID, const bdNChar8* licenseKey);
    bdBitBufferRef makeAuthRequestForDedicatedServer(const unsigned int);
    bdBitBufferRef makeAuthRequestForDedicatedServerRSA(const unsigned int);
    bdBitBufferRef makeAuthAccountForHost(const unsigned int, char*, const unsigned int);
    bdLobbyErrorCode handleReply(bdUByte8 replyType, bdBitBufferRef reply);
    static void createAuthCookie(bdUByte8* cookie);
    bdBitBufferRef makeAuthForSteam(const bdUInt titleID, const bdNChar8* ticket, const bdUInt ticketSize);
    bdBool handleSteamReply(bdBitBufferRef buffer);
};
