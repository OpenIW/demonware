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
