// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

bdLobbyService::bdLobbyService()
    : bdLobbyConnectionListener(), m_taskManager(NULL), m_profiles(NULL), m_messaging(NULL), m_matchMaking(NULL), m_statsManager(NULL), m_friendsManager(NULL),
    m_teamsManager(NULL), m_storageManager(NULL), m_contentUnlockManager(NULL), m_keyArchive(NULL), m_counter(NULL), m_group(NULL), m_contentStreamingManager(NULL),
    m_pooledStorageManager(NULL), m_tags(NULL), m_voteRankManager(NULL), m_twitter(NULL), m_facebook(NULL), m_linkCode(NULL), m_antiCheat(NULL), m_DML(NULL),
    m_crux(NULL), m_activityTracker(NULL), m_titleID(0), m_eventHandler(NULL), m_lobbyConnection(), m_authInfo(), m_errorCode(BD_NO_ERROR)
{
}

bdLobbyService::~bdLobbyService()
{
    cleanup();
}

bdBool bdLobbyService::connect(bdAddr lobbyServiceAddr, const bdAuthInfo* authInfo, bdUInt maxSendBufSize, bdUInt maxRecvBufSize, const bdBool useEncryption)
{
    if (!authInfo)
    {
        bdLogError("lobby service", "No authentication info specified");
        return false;
    }
    if (*m_lobbyConnection)
    {
        m_lobbyConnection->disconnect();
        cleanup();
    }
    m_encryptedConnection = useEncryption;
    bdMemcpy(&m_authInfo, authInfo, sizeof(m_authInfo));
    m_lobbyConnection = new bdLobbyConnection(bdCommonAddrRef(new bdCommonAddr(lobbyServiceAddr)), maxSendBufSize, maxRecvBufSize, this);
    m_lobbyConnection->connect(m_authInfo);
    return true;
}

bdBool bdLobbyService::connect(bdAddr lobbyServiceAddr, const bdAuthInfo* authInfo, const bdBool useEncryption)
{
    return connect(bdAddr(lobbyServiceAddr), authInfo, 0xFFFF, 0xFFFF, useEncryption);
}

void bdLobbyService::disconnect()
{
    if (*m_lobbyConnection)
    {
        m_lobbyConnection->disconnect();
    }
    m_titleID = 0;
    cleanup();
}

void bdLobbyService::pump()
{
    bdUInt32 errorCode;
    bdUInt64 connectionID;

    if (*m_lobbyConnection)
    {
        bdByteBufferRef message;
        bdUByte8 messageType = 0;
        for (;;)
        {
            if (!m_lobbyConnection->getMessageToDispatch(messageType, message))
            {
                break;
            }
            switch (messageType)
            {
            case 1:
                bdLogInfo("lobby service", "Received message of type: BD_LOBBY_SERVICE_TASK_REPLY");
                if (m_taskManager)
                {
                    m_taskManager->handleLSGTaskReply(bdByteBufferRef(message));
                }
                else
                {
                    bdLogWarn("lobby service", "No task manager.");
                }
                break;

            case 2:
                bdLogInfo("lobby service", "Received message of type: BD_LOBBY_SERVICE_PUSH_MESSAGE");
                handlePushMessage(bdByteBufferRef(message));
                break;
            case 3:
                if (message->readUInt32(errorCode))
                {
                    bdLogInfo("lobby service", "Received LSG error: %u", errorCode);
                }
                else
                {
                    bdLogWarn("lobby service", "Received LSG error message, but cannot read out errorCode.");
                }
                break;
            case 4:
                if (message->readUInt64(connectionID))
                {
                    if (m_taskManager)
                    {
                        bdLogInfo("lobby service", "Received LSG connection ID:%llu", connectionID);
                        m_taskManager->setConnectionID(connectionID);
                    }
                    else
                    {
                        bdLogWarn("lobby service", "No task manager to assign connection ID to.");
                    }
                }
                else
                {
                    bdLogWarn("lobby service", "Received LSG connection ID header, but couldn't read ID.");
                }
                break;
            case 5:
                bdLogInfo("lobby service", "Received message of type: BD_LSG_SERVICE_TASK_REPLY");
                if (m_taskManager && message.notNull())
                {
                    m_taskManager->handleTaskReply(bdByteBufferRef(message));
                }
                else
                {
                    bdLogWarn("lobby service", "Null message or no task manager.");
                }
                break;
            default:
                bdLogWarn("lobby service", "Received unknown message type: %u.", messageType);
                break;
            }
        }
        if (m_taskManager)
        {
            m_taskManager->cleanUpAsyncState();
        }
    }
}

bdLobbyConnection::Status bdLobbyService::getStatus()
{
    if (*m_lobbyConnection)
    {
        return m_lobbyConnection->getStatus();
    }
    return bdLobbyConnection::BD_NOT_CONNECTED;
}

void bdLobbyService::registerEventHandler(bdLobbyEventHandler* eventHandler)
{
    m_eventHandler = eventHandler;
}

bdMessaging* bdLobbyService::getMessaging()
{
    return m_messaging;
}

bdMatchMaking* bdLobbyService::getMatchMaking()
{
    if (*m_lobbyConnection && m_lobbyConnection->getStatus() == bdLobbyConnection::BD_CONNECTED)
    {
        if (!m_matchMaking)
        {
            m_matchMaking = new bdMatchMaking(m_taskManager);
        }
        return m_matchMaking;
    }
    bdLogWarn("lobby service", "Not connected.");
    return NULL;
}

bdProfiles* bdLobbyService::getProfiles()
{
    if (*m_lobbyConnection && m_lobbyConnection->getStatus() == bdLobbyConnection::BD_CONNECTED)
    {
        if (!m_profiles)
        {
            m_profiles = new bdProfiles(m_taskManager);
        }
        return m_profiles;
    }
    bdLogWarn("lobby service", "Not connected.");
    return NULL;
}

bdStats* bdLobbyService::getStats()
{
    if (*m_lobbyConnection && m_lobbyConnection->getStatus() == bdLobbyConnection::BD_CONNECTED)
    {
        if (!m_statsManager)
        {
            m_statsManager = new bdStats(m_taskManager);
        }
        return m_statsManager;
    }
    bdLogWarn("lobby service", "Not connected.");
    return NULL;
}

bdTeams* bdLobbyService::getTeams()
{
    return m_teamsManager;
}

bdFriends* bdLobbyService::getFriends()
{
    return m_friendsManager;
}

bdStorage* bdLobbyService::getStorage()
{
    if (*m_lobbyConnection && m_lobbyConnection->getStatus() == bdLobbyConnection::BD_CONNECTED)
    {
        if (!m_storageManager)
        {
            m_storageManager = new bdStorage(m_taskManager);
        }
        return m_storageManager;
    }
    bdLogWarn("lobby service", "Not connected.");
    return NULL;
}

bdContentUnlock* bdLobbyService::getContentUnlock()
{
    return m_contentUnlockManager;
}

bdTitleUtilities* bdLobbyService::getTitleUtilities()
{
    if (*m_lobbyConnection && m_lobbyConnection->getStatus() == bdLobbyConnection::BD_CONNECTED)
    {
        if (!m_titleUtilitiesManager)
        {
            m_titleUtilitiesManager = new bdTitleUtilities(m_taskManager);
        }
        return m_titleUtilitiesManager;
    }
    bdLogWarn("lobby service", "Not connected.");
    return NULL;
}

bdKeyArchive* bdLobbyService::getKeyArchive()
{
    return m_keyArchive;
}

bdCounter* bdLobbyService::getCounter()
{
    if (*m_lobbyConnection && m_lobbyConnection->getStatus() == bdLobbyConnection::BD_CONNECTED)
    {
        if (!m_counter)
        {
            m_counter = new bdCounter(m_taskManager);
        }
        return m_counter;
    }
    bdLogWarn("lobby service", "Not connected.");
    return NULL;
}

bdGroup* bdLobbyService::getGroup()
{
    if (*m_lobbyConnection && m_lobbyConnection->getStatus() == bdLobbyConnection::BD_CONNECTED)
    {
        if (!m_group)
        {
            m_group = new bdGroup(m_taskManager);
        }
        return m_group;
    }
    bdLogWarn("lobby service", "Not connected.");
    return NULL;
}

bdContentStreaming* bdLobbyService::getContentStreaming()
{
    if (*m_lobbyConnection && m_lobbyConnection->getStatus() == bdLobbyConnection::BD_CONNECTED)
    {
        if (!m_contentStreamingManager)
        {
            m_contentStreamingManager = new bdContentStreaming(m_taskManager);
        }
        return m_contentStreamingManager;
    }
    bdLogWarn("lobby service", "Not connected.");
    return NULL;
}

void bdLobbyService::deleteContentStreaming()
{
    if (m_contentStreamingManager)
    {
        m_contentStreamingManager->~bdContentStreaming();
        m_contentStreamingManager = NULL;
    }
}

bdPooledStorage* bdLobbyService::getPooledStorage()
{
    if (*m_lobbyConnection && m_lobbyConnection->getStatus() == bdLobbyConnection::BD_CONNECTED)
    {
        if (!m_pooledStorageManager)
        {
            m_pooledStorageManager = new bdPooledStorage(m_taskManager);
        }
        return m_pooledStorageManager;
    }
    bdLogWarn("lobby service", "Not connected.");
    return NULL;
}

void bdLobbyService::deletePooledStorage()
{
    if (m_pooledStorageManager)
    {
        m_pooledStorageManager->~bdPooledStorage();
        m_pooledStorageManager = NULL;
    }
}

bdTags* bdLobbyService::getTags()
{
    if (*m_lobbyConnection && m_lobbyConnection->getStatus() == bdLobbyConnection::BD_CONNECTED)
    {
        if (!m_tags)
        {
            m_tags = new bdTags(m_taskManager);
        }
        return m_tags;
    }
    bdLogWarn("lobby service", "Not connected.");
    return NULL;
}

bdVoteRank* bdLobbyService::getVoteRank()
{
    if (*m_lobbyConnection && m_lobbyConnection->getStatus() == bdLobbyConnection::BD_CONNECTED)
    {
        if (!m_voteRankManager)
        {
            m_voteRankManager = new bdVoteRank(m_taskManager);
        }
        return m_voteRankManager;
    }
    bdLogWarn("lobby service", "Not connected.");
    return NULL;
}

bdTwitter* bdLobbyService::getTwitter()
{
    return m_twitter;
}

bdFacebook* bdLobbyService::getFacebook()
{
    return m_facebook;
}

bdLinkCode* bdLobbyService::getLinkCode()
{
    return m_linkCode;
}

bdAntiCheat* bdLobbyService::getAntiCheat()
{
    return m_antiCheat;
}

bdDML* bdLobbyService::getDML()
{
    return m_DML;
}

bdCRUX* bdLobbyService::getCRUX()
{
    return m_crux;
}

bdActivityTracker* bdLobbyService::getActivityTracker()
{
    return m_activityTracker;
}

bdRemoteTaskManager* bdLobbyService::getTaskManager()
{
    return m_taskManager;
}

bdLobbyErrorCode bdLobbyService::getErrorCode()
{
    return m_errorCode;
}

bdUInt bdLobbyService::getTitleID()
{
    return m_titleID;
}

void bdLobbyService::onConnect(bdLobbyConnectionRef connection)
{
    bdByteBufferRef byteBuffer(new bdByteBuffer(8, false));
    byteBuffer->writeUInt32(200);
    byteBuffer->writeUInt32(connection->getReceiveBufferSize());
    connection->sendRaw(&byteBuffer, 8);
    bdBitBufferRef bitBuffer(new bdBitBuffer(8, true));
    bitBuffer->setTypeCheck(false);
    bitBuffer->writeBool(true);
    bitBuffer->setTypeCheck(true);
    bitBuffer->writeUInt32(m_authInfo.m_titleID);
    if (m_encryptedConnection)
    {
        bitBuffer->writeUInt32(m_authInfo.m_IVSeed);
    }
    else
    {
        bitBuffer->writeUInt32(0);
    }
    bitBuffer->writeBits(m_authInfo.m_data, sizeof(m_authInfo.m_data) * CHAR_BIT);
    connection->send(bitBuffer->getData(), bitBuffer->getDataSize(), false);
    m_taskManager = new bdRemoteTaskManager(bdLobbyConnectionRef(connection), m_encryptedConnection);
    bdLogInfo("lobby service", "Connected to MatchMaking Service.");
}

void bdLobbyService::onDisconnect(bdLobbyConnectionRef connection)
{
    bdLogInfo("lobby service", "Disconnected from MatchMaking Service.");
}

void bdLobbyService::cleanup()
{
    if (m_taskManager)
    {
        delete m_taskManager;
    }
    if (m_profiles)
    {
        delete m_profiles;
    }
    if (m_messaging)
    {
        delete m_messaging;
    }
    if (m_matchMaking)
    {
        delete m_matchMaking;
    }
    if (m_statsManager)
    {
        delete m_statsManager;
    }
    if (m_friendsManager)
    {
        delete m_friendsManager;
    }
    if (m_teamsManager)
    {
        delete m_teamsManager;
    }
    if (m_storageManager)
    {
        delete m_storageManager;
    }
    if (m_contentUnlockManager)
    {
        delete m_contentUnlockManager;
    }
    if (m_titleUtilitiesManager)
    {
        delete m_titleUtilitiesManager;
    }
    if (m_keyArchive)
    {
        delete m_keyArchive;
    }
    if (m_counter)
    {
        delete m_counter;
    }
    if (m_group)
    {
        delete m_group;
    }
    if (m_contentStreamingManager)
    {
        delete m_contentStreamingManager;
    }
    if (m_pooledStorageManager)
    {
        delete m_pooledStorageManager;
    }
    if (m_tags)
    {
        delete m_tags;
    }
    if (m_voteRankManager)
    {
        delete m_voteRankManager;
    }
    if (m_twitter)
    {
        delete m_twitter;
    }
    if (m_facebook)
    {
        delete m_facebook;
    }
    if (m_linkCode)
    {
        delete m_linkCode;
    }
    if (m_antiCheat)
    {
        delete m_antiCheat;
    }
    if (m_DML)
    {
        delete m_DML;
    }
    if (m_activityTracker)
    {
        delete m_activityTracker;
    }
    m_lobbyConnection = (bdLobbyConnection*)NULL;
}

void bdLobbyService::handlePushMessage(bdByteBufferRef message)
{
    bdUInt msgType = 0;
    bdBool ok = message->readUInt32(msgType);
    if (!ok)
    {
        bdLogWarn("lobby service", "Failed to deserialzie the EventType of Push Message");
        return;
    }
    if (msgType == 20)
    {
        m_taskManager->handleAsyncResult(bdByteBufferRef(message));
    }
    else if (m_eventHandler)
    {
        switch (msgType)
        {
        case 1:
            m_eventHandler->onNewNotification();
            break;
        case 2:
            m_eventHandler->onFriendshipProposal();
            break;
        case 3:
            m_eventHandler->onTeamProposal();
            break;
        case 4:
        case 5:
        {
            bdUInt64 friendUID;
            bdNChar8 friendName[65];
            ok = message->readUInt64(friendUID);
            bdMemset(friendName, 0, sizeof(friendName));
            ok = ok == message->readString(friendName, 64);
            if (ok)
            {
                m_eventHandler->onPlayerStatusUpdate(friendUID, friendName, msgType == 4);
            }
            break;
        }
        case 6:
        {
            bdSessionInvite invite;
            ok = invite.deserialize(bdByteBufferRef(message));
            if (ok)
            {
                m_eventHandler->onSessionInvite(&invite);
            }
            else
            {
                bdLogWarn("lobby service", "Failed to read session invite.");
            }
            break;
        }
        case 7:
        case 8:
        {
            bdUInt64 senderUID = 0;
            bdNChar8 senderName[64];
            ok = message->readUInt64(senderUID);
            ok = ok == message->readString(senderName, sizeof(senderName));

            bdUInt64 channelID = 0;
            ok = ok == message->readUInt64(channelID);

            bdUInt32 msgLength = 1024;
            bdUByte8 msg[1024];
            bdMemset(msg, 0, msgLength);
            ok = ok == message->readBlob(msg, msgLength);

            if (ok)
            {
                m_eventHandler->onChatChannelMessage(channelID, senderUID, senderName, msg, msgLength);
            }
            break;
        }
        case 9:
        case 10:
        {
            bdUInt64 userID;
            bdNChar8 userName[64];
            bdUInt64 channelID;
            ok = message->readUInt64(userID);
            ok = ok == message->readString(userName, 64);
            ok = ok == message->readUInt64(channelID);
            if (ok)
            {
                m_eventHandler->onChatChannelUpdate(channelID, userID, userName, msgType == 9);
            }
            break;
        }
        case 11:
        case 12:
        {
            bdUInt64 senderUID = 0;
            ok = message->readUInt64(senderUID);

            bdNChar8 senderName[65];
            bdMemset(senderName, 0, sizeof(senderName));
            ok = ok == message->readString(senderName, sizeof(senderName) - 1);

            bdUInt64 teamID = 0;
            ok = ok == message->readUInt64(teamID);

            bdNChar8 teamName[65];
            bdMemset(teamName, 0, sizeof(teamName));
            ok = ok == message->readString(teamName, sizeof(teamName) - 1);

            if (ok)
            {
                m_eventHandler->onTeamMemberStatusUpdate(senderUID, senderName, teamID, teamName, msgType == 11);
            }
            break;
        }
        
        case 13:
        {
            bdUInt64 senderID = 0;
            ok = message->readUInt64(senderID);

            bdNChar8 senderName[64];
            bdMemset(senderName, 0, sizeof(senderName));
            ok = ok == message->readString(senderName, sizeof(senderName));
            
            bdUInt richPresenceSize = 1024;
            bdUByte8 richPresence[1024];
            ok = ok == message->readBlob(richPresence, richPresenceSize);

            if (ok)
            {
                m_eventHandler->onRichPresenceUpdate(senderID, senderName, richPresence, richPresenceSize, 0);
            }
            break;
        }

        case 14:
        {
            bdUInt64 teamID = 0;
            ok = message->readUInt64(teamID);

            bdUInt64 senderID = 0;
            ok = ok == message->readUInt64(senderID);

            bdNChar8 senderName[64];
            bdMemset(senderName, 0, sizeof(senderName));
            ok = ok == message->readString(senderName, sizeof(senderName));

            bdUInt richPresenceSize = 1024;
            bdUByte8 richPresence[1024];
            ok = ok == message->readBlob(richPresence, richPresenceSize);

            if (ok)
            {
                m_eventHandler->onRichPresenceUpdate(senderID, senderName, richPresence, richPresenceSize, teamID);
            }
            break;
        }

        case 15:
        {
            bdUInt64 senderID = 0;
            ok = message->readUInt64(senderID);

            bdNChar8 senderName[64];
            bdMemset(senderName, 0, sizeof(senderName));
            ok = ok == message->readString(senderName, sizeof(senderName));

            bdUInt msgLength = 1024;
            bdUByte8 msg[1024];
            bdMemset(msg, 0, sizeof(msg));
            ok = ok == message->readBlob(msg, msgLength);

            if (ok)
            {
                m_eventHandler->onInstantMessage(senderID, senderName, msg, msgLength, 0);
            }
            break;
        }
        
        case 16:
        {
            bdUInt64 senderID = 0;
            ok = message->readUInt64(senderID);

            bdNChar8 senderName[64];
            bdMemset(senderName, 0, sizeof(senderName));
            ok = ok == message->readString(senderName, sizeof(senderName));

            bdUInt64 teamID = 0;
            ok = ok == message->readUInt64(teamID);

            bdNChar8 teamName[65];
            bdMemset(teamName, 0, sizeof(teamName));
            ok = ok == message->readString(teamName, sizeof(teamName) - 1);

            bdUInt msgLength = 1024;
            bdUByte8 msg[1024];
            bdMemset(msg, 0, sizeof(msg));
            ok = ok == message->readBlob(msg, msgLength);

            if (ok)
            {
                m_eventHandler->onInstantMessage(senderID, senderName, msg, msgLength, teamID);
            }
        }

        case 17:
        {
            bdUInt64 userID = 0;
            ok = message->readUInt64(userID);

            bdNChar8 senderName[64];
            bdMemset(senderName, 0, sizeof(senderName));
            ok = ok == message->readString(senderName, sizeof(senderName));

            bdSessionID sessionID;
            bdUInt length = 8;
            ok = ok == message->readBlob(sessionID.m_sessionID.ab, length);

            if (ok)
            {
                m_eventHandler->onNotifyLeave(userID, &bdSessionID(&sessionID));
            }
            else
            {
                bdLogWarn("lobby service", "Failed to read sessionID for session invite.");
            }
            break;
        }

        case 18:
            m_eventHandler->onNewMail();
            break;
            
        case 19:
            // anticheat challenge, not used in BO1. So this will be ignored for now.
            break;

        case 20:
            break;

        case 21:
        {
            bdUInt64 senderUID = 0;
            ok = message->readUInt64(senderUID);

            bdNChar8 senderName[64];
            bdMemset(senderName, 0, sizeof(senderName));
            ok = ok == message->readString(senderName, sizeof(senderName));

            bdUInt msgLength = 4096;
            bdUByte8 msg[4096];
            bdMemset(msg, 0, sizeof(msg));
            ok = ok == message->readBlob(msg, msgLength);

            if (ok)
            {
                m_eventHandler->onGlobalInstantMessage(senderUID, senderName, msg, msgLength);
            }
            break;
        }

        case 29:
        {
            bdUInt64 recipientID = 0;
            ok = message->readUInt64(recipientID);

            if (ok)
            {
                m_eventHandler->onMultipleLogon(recipientID);
            }
            break;
        }
        
        case 30:
        {
            bdUInt64 recipientID = 0;
            ok = message->readUInt64(recipientID);

            if (ok)
            {
                m_eventHandler->onPlayerBanned(recipientID);
            }
            break;
        }
        default:
            break;
        }
    }
}
