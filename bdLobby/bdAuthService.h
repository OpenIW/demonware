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
protected:
    bdFastArray<bdNChar8[64]> m_usernamesForLicenseResult;
    bdReference<bdLobbyConnection> m_connection;
    bdUInt m_titleID;
    bdReference<bdCommonAddr> m_authServiceAddr;
    bdAuthService::bdStatus m_status;
    bdReference<bdBitBuffer> m_request;
    bdAuthInfo m_authInfo;
    bdAuthTicket m_authTicket;
    bdNChar8 m_steamCookieKey[88];
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
    bdAuthService(bdUInt titleID, const bdAddr* addr);
    ~bdAuthService();
    bdBool createAccount(char*, char*, char*);
    bdBool changePassword(char*, char*, char*);
    bdBool resetAccount(char*, char*, char*);
    bdBool deleteAccount(char*, char*);
    bdBool migrateAccount(char*, const unsigned int, char*);
    bdBool authorizeAccount(char*, const unsigned int, char*);
    bdBool authorizeAccount(char*, char*);
    bdBool authorizeAnonymous();
    bdBool authorizeDedicatedHost(char*);
    bdBool authorizeDedicatedServer();
    bdBool authorizeDedicatedServerRSA();
    bdBool getUsernamesForLicense(char*);
    bdNChar8* getSteamRequestData(char*, unsigned int*);
    bdBool authorizeSteamTicket(unsigned int*, const unsigned int);
    void createSteamCookie(char*);
    void setSteamCookie(char*);
    bdAuthService::bdStatus getStatus();
    bdLobbyErrorCode getErrorCode();
    bdAuthInfo* getAuthInfo();
    bdAuthTicket* getAuthTicket();
protected:
    void startTask();
    void onConnect(bdLobbyConnectionRef);
    void onDisconnect(bdLobbyConnectionRef);
    void onConnectFailed(bdLobbyConnectionRef);
    bdBitBufferRef makeCreateAccount(const unsigned int, char*, char*, char*);
    bdBitBufferRef makeGetUsernamesForLicense(const unsigned int, char*);
    bdBitBufferRef makeChangeUserKey(const unsigned int, char*, char*, char*);
    bdBitBufferRef makeResetAccount(const unsigned int, char*, char*, char*);
    bdBitBufferRef makeDeleteAccount(const unsigned int, char*, char*);
    bdBitBufferRef makeMigrateAccount(const unsigned int, char*, const unsigned int, char*);
    bdBitBufferRef makeAuthAccountForService(const unsigned int, char*);
    bdBitBufferRef makeAuthAnonymousForService(const unsigned int);
    bdBitBufferRef makeAuthHostForService(const unsigned int, char*);
    bdBitBufferRef makeAuthRequestForDedicatedServer(const unsigned int);
    bdBitBufferRef makeAuthRequestForDedicatedServerRSA(const unsigned int);
    bdBitBufferRef makeAuthAccountForHost(const unsigned int, char*, const unsigned int);
    bdLobbyErrorCode handleReply(unsigned int, bdBitBufferRef);
    void createAuthCookie(char*);
    bdReference<bdBitBuffer> makeAuthForSteam(const unsigned int, unsigned int*, unsigned int);
    bdBool handleSteamReply(bdBitBufferRef);
};
