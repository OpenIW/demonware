// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdLobbyEventHandler
{
public:
    bdLobbyEventHandler();
    virtual ~bdLobbyEventHandler();
    virtual void onPlayerStatusUpdate(unsigned long long, char*, unsigned char);
    virtual void onTeamMemberStatusUpdate(unsigned long long, char*, unsigned long long, char*, unsigned char);
    virtual void onRichPresenceUpdate(unsigned long long, char*, void*, unsigned int, unsigned long long);
    virtual void onChatChannelUpdate(unsigned long long, unsigned long long, char*, unsigned char);
    virtual void onChatChannelMessage(unsigned long long, unsigned long long, char*, void*, unsigned int);
    virtual void onInstantMessage(unsigned long long, char*, void*, unsigned int, unsigned long long);
    virtual void onSessionInvite(bdSessionInvite& invite);
    virtual void onNotifyLeave(bdUInt64 userID, bdSessionID sessionID);
    virtual void onFriendshipProposal();
    virtual void onTeamProposal();
    virtual void onNewNotification();
    virtual void onNewMail();
    virtual void onMultipleLogon(bdUInt64 userID);
    virtual void onChallengesReceived(bdReference<bdAntiCheatChallenges>);
    virtual void onPlayerBanned(bdUInt64 userID);
    virtual void onGlobalInstantMessage(bdUInt64 senderID, bdNChar8* senderName, void* message, bdUInt messageSize);
};
