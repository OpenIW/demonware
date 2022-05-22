// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "bdNet/bdNet.h"

#include "bdLobbyErrorCode.h"
#include "bdLobbyErrorCodes.h"
#include "bdLobbyConnectionListener.h"
#include "bdLobbyCommon/bdAuthCommon.h"
#include "bdLobbyCommon/bdTaskResult.h"
#include "bdLobbyCommon/bdTaskResultProcessor.h"
#include "bdRemoteTaskManager/bdTaskByteBuffer.h"
#include "bdRemoteTaskManager/bdRemoteTask.h"
#include "bdAuthTicket.h"
#include "bdAuthService.h"
#include "bdLobbyConnection.h"
#include "bdRemoteTaskManager/bdRemoteTaskManager.h"

// AntiCheat headers
#include "bdAntiCheat/bdAntiCheatChallengeParam.h"
#include "bdAntiCheat/bdAntiCheatChallenge.h"
#include "bdAntiCheat/bdAntiCheatChallenges.h"
#include "bdAntiCheat/bdAntiCheatResponses.h"
#include "bdAntiCheat/bdAntiCheat.h"

#include "bdContentStreaming/bdFileID.h"

// Tag headers
#include "bdTags/bdTag.h"
#include "bdTags/bdTags.h"

// ContentStreaming headers
#include "bdContentStreaming/bdFileMetadata.h"
#include "bdContentStreaming/bdURL.h"
#include "bdContentStreaming/bdDownloadInterceptor.h"
#include "bdContentStreaming/bdUploadInterceptor.h"

// HTTPWrapper headers
#include "bdHTTPWrapper/bdHTTPBuffer.h"
#include "bdHTTPWrapper/bdHTTPClient.h"
#include "bdHTTPWrapper/bdHTTPWrapperBase.h"
#include "bdHTTPWrapper/bdHTTPWrapper.h"

// Last of ContentStreaming headers
#include "bdContentStreaming/bdSummaryMetahandler.h"
#include "bdContentStreaming/bdPrecopyResult.h"
#include "bdContentStreaming/bdContentStreamingBase.h"
#include "bdContentStreaming/bdContentStreaming.h"

// Profile headers
#include "bdProfiles/bdProfileInfo.h"
#include "bdProfiles/bdProfiles.h"

// Counter headers
#include "bdCounter/bdCounterValue.h"
#include "bdCounter/bdCounter.h"

// DML headers
#include "bdDML/bdDML.h"

// Friend headers
#include "bdFriends/bdFriends.h"
#include "bdFriends/bdRichPresence.h"

// Group headers
#include "bdGroup/bdGroupID.h"
#include "bdGroup/bdGroupCount.h"
#include "bdGroup/bdGroup.h"

// KeyArchive headers
#include "bdKeyArchive/bdKeyValuePair.h"
#include "bdKeyArchive/bdKeyArchive.h"

// Messaging headers
#include "bdMessaging/bdLobbyMessageType.h"
#include "bdMessaging/bdMailHeader.h"
#include "bdMessaging/bdMailBody.h"
#include "bdMessaging/bdNotification.h"
#include "bdMessaging/bdMessaging.h"

// PooledStorage headers
#include  "bdPooledStorage/bdPooledStorage.h"

// Stats headers
#include "bdStats/bdStats.h"
#include "bdStats/bdStatsInfo.h"

// VoteRank headers
#include "bdVoteRank/bdRatingInfo.h"
#include "bdVoteRank/bdVoteRank.h"
#include "bdVoteRank/bdVoteRankStatsInfo.h"

// Storage headers
#include "bdStorage/bdFileInfo.h"
#include "bdStorage/bdFileData.h"
#include "bdStorage/bdStorage.h"

// TitleUtilities headers
#include "bdTitleUtilities/bdVerifyString.h"
#include "bdTitleUtilities/bdTimestamp.h"
#include "bdTitleUtilities/bdOnlineUserInfo.h"
#include "bdTitleUtilities/bdTitleUtilities.h"

// MatchMaking headers
#include "bdMatchmaking/bdSessionID.h"
#include "bdMatchmaking/bdSessionParams.h"
#include "bdMatchmaking/bdSessionInvite.h"
#include "bdMatchmaking/bdPerformanceValue.h"
#include "bdMatchmaking/bdPagingToken.h"
#include "bdMatchmaking/bdMatchmakingInfo.h"
#include "bdMatchmaking/bdMatchmaking.h"

// EventHandler headers
#include "bdLobbyEventHandler.h"

#include "bdLobbyService.h"
