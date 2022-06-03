// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

bdLobbyEventHandler::bdLobbyEventHandler()
{
}

bdLobbyEventHandler::~bdLobbyEventHandler()
{
}

void bdLobbyEventHandler::onPlayerStatusUpdate(unsigned long long, char*, unsigned char)
{
}

void bdLobbyEventHandler::onTeamMemberStatusUpdate(unsigned long long, char*, unsigned long long, char*, unsigned char)
{
}

void bdLobbyEventHandler::onRichPresenceUpdate(unsigned long long, char*, void*, unsigned int, unsigned long long)
{
}

void bdLobbyEventHandler::onChatChannelUpdate(unsigned long long, unsigned long long, char*, unsigned char)
{
}

void bdLobbyEventHandler::onChatChannelMessage(unsigned long long, unsigned long long, char*, void*, unsigned int)
{
}

void bdLobbyEventHandler::onInstantMessage(unsigned long long, char*, void*, unsigned int, unsigned long long)
{
}

void bdLobbyEventHandler::onSessionInvite(bdSessionInvite& invite)
{
}

void bdLobbyEventHandler::onNotifyLeave(bdUInt64 userID, bdSessionID sessionID)
{
}

void bdLobbyEventHandler::onFriendshipProposal()
{
}

void bdLobbyEventHandler::onTeamProposal()
{
}

void bdLobbyEventHandler::onNewNotification()
{
}

void bdLobbyEventHandler::onNewMail()
{
}

void bdLobbyEventHandler::onMultipleLogon(bdUInt64 userID)
{
}

void bdLobbyEventHandler::onChallengesReceived(bdReference<bdAntiCheatChallenges>)
{
}

void bdLobbyEventHandler::onPlayerBanned(bdUInt64 userID)
{
}

void bdLobbyEventHandler::onGlobalInstantMessage(bdUInt64 senderID, bdNChar8* senderName, void* message, bdUInt messageSize)
{
}
