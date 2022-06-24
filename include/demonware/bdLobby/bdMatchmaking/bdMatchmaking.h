// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdMatchMaking
{
protected:
    bdRemoteTaskManager* m_remoteTaskManager;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdMatchMaking(bdRemoteTaskManager* remoteTaskManager);
    ~bdMatchMaking();
    bdRemoteTaskRef createSession(bdMatchMakingInfo* sessionInfo, bdSessionID* sessionCredentials);
    bdRemoteTaskRef updateSession(const bdSessionID& sessionID, bdMatchMakingInfo* const sessionInfo);
    bdRemoteTaskRef updateSessionPlayers(const bdSessionID& sessionID, bdMatchMakingInfo* const sessionInfo, bdUInt32 numPlayers);
    bdRemoteTaskRef deleteSession(const bdSessionID& sessionID);
    bdRemoteTaskRef findSessionFromID(const bdSessionID& sessionID, bdMatchMakingInfo* const sessionInfo);
    bdRemoteTaskRef findSessions(const bdUInt queryID, const bdUInt startIndexDEPRECATED, const bdUInt maxNumResults, bdSessionParams* sessionParams, bdMatchMakingInfo* results);
    bdRemoteTaskRef findSessionsPaged(const bdUInt queryID, bdSessionParams* sessionParams, bdPagingToken* token, bdMatchMakingInfo* results);
    bdRemoteTaskRef findSessionsByEntityIDs(const bdUInt64* const entityIDs, const bdUInt numEntityIDs, bdMatchMakingInfo* results);
    bdRemoteTaskRef inviteToSession(const bdSessionID& sessionID, const bdUInt64* userIDs, const bdUInt numUsers, const void* const attachment, const bdUInt attachmentSize);
    bdRemoteTaskRef notifyJoin(const bdSessionID& sessionID, const bdUInt64* users, const bdUInt numUsers);
    bdRemoteTaskRef notifyLeave(const bdSessionID& sessionID, const bdUInt64* users, const bdUInt numUsers);
    bdRemoteTaskRef submitPerformance(const bdUInt gameType, bdPerformanceValue* performanceValues, const bdUInt numValues);
    bdRemoteTaskRef getPerformanceValues(const bdUInt64* const entityIDs, const bdUInt numEntityIDs, const bdUInt gameType, bdPerformanceValue* results);
    bdRemoteTaskRef getSessionInvites(const bdUInt32 startIndex, const bdUInt32 maxNumResults, bdSessionInvite* results);
    bdRemoteTaskRef startNotifyTask(const bdSessionID& sessionID, const bdUInt64* users, const bdUInt numUsers, const bdUByte8 taskType);
};
