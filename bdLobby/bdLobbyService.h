// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

// Temp: Declare unused classes in BO1
class bdTeams;
class bdTwitter;
class bdFacebook;
class bdLinkCode;
class bdCRUX;
class bdActivityTracker;
class bdContentUnlock;

class bdLobbyService : public bdLobbyConnectionListener
{
protected:
    bdRemoteTaskManager* m_taskManager;
    bdProfiles* m_profiles;
    bdMessaging* m_messaging;
    bdMatchMaking* m_matchMaking;
    bdStats* m_statsManager;
    bdFriends* m_friendsManager;
    bdTeams* m_teamsManager;
    bdStorage* m_storageManager;
    bdContentUnlock* m_contentUnlockManager;
    bdTitleUtilities* m_titleUtilitiesManager;
    bdKeyArchive* m_keyArchive;
    bdCounter* m_counter;
    bdGroup* m_group;
    bdContentStreaming* m_contentStreamingManager;
    bdPooledStorage* m_pooledStorageManager;
    bdTags* m_tags;
    bdVoteRank* m_voteRankManager;
    bdTwitter* m_twitter;
    bdFacebook* m_facebook;
    bdLinkCode* m_linkCode;
    bdAntiCheat* m_antiCheat;
    bdDML* m_DML;
    bdCRUX* m_crux;
    bdActivityTracker* m_activityTracker;
    bdUInt m_titleID;
    bdLobbyEventHandler* m_eventHandler;
    bdLobbyConnectionRef m_lobbyConnection;
    bdAuthInfo m_authInfo;
    bdBool m_encryptedConnection;
    bdLobbyErrorCode m_errorCode;
public:
    bdLobbyService();
    ~bdLobbyService();
    bdBool connect(bdAddr lobbyServiceAddr, const bdAuthInfo* authInfo, bdUInt maxSendBufSize, bdUInt maxRecvBufSize, const bdBool useEncryption);
    bdBool connect(bdAddr lobbyServiceAddr, const bdAuthInfo* authInfo, const bdBool useEncryption);
    void disconnect();
    void pump();
    bdLobbyConnection::Status getStatus();
    void registerEventHandler(bdLobbyEventHandler* eventHandler);
    bdMessaging* getMessaging();
    bdMatchMaking* getMatchMaking();
    bdProfiles* getProfiles();
    bdStats* getStats();
    bdTeams* getTeams();
    bdFriends* getFriends();
    bdStorage* getStorage();
    bdContentUnlock* getContentUnlock();
    bdTitleUtilities* getTitleUtilities();
    bdKeyArchive* getKeyArchive();
    bdCounter* getCounter();
    bdGroup* getGroup();
    bdContentStreaming* getContentStreaming();
    void deleteContentStreaming();
    bdPooledStorage* getPooledStorage();
    void deletePooledStorage();
    bdTags* getTags();
    bdVoteRank* getVoteRank();
    bdTwitter* getTwitter();
    bdFacebook* getFacebook();
    bdLinkCode* getLinkCode();
    bdAntiCheat* getAntiCheat();
    bdDML* getDML();
    bdCRUX* getCRUX();
    bdActivityTracker* getActivityTracker();
    bdRemoteTaskManager* getTaskManager();
    bdLobbyErrorCode getErrorCode();
    bdUInt getTitleID();
protected:
    void onConnect(bdLobbyConnectionRef connection);
    void onDisconnect(bdLobbyConnectionRef connection);
    void cleanup();
    void handlePushMessage(bdByteBufferRef message);
};