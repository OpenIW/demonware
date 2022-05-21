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
};