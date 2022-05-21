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
    bdRemoteTaskRef createSession(bdMatchMakingInfo*, const bdSessionID*);
    bdRemoteTaskRef updateSession(bdSessionID&, const bdMatchMakingInfo*);
    bdRemoteTaskRef updateSessionPlayers(bdSessionID*, const bdMatchMakingInfo*, unsigned int);
    bdRemoteTaskRef deleteSession(bdSessionID*);
    bdRemoteTaskRef findSessionFromID(bdSessionID*, const bdMatchMakingInfo*);
    bdRemoteTaskRef findSessions(const unsigned int, const unsigned int, const unsigned int, bdSessionParams*, bdMatchMakingInfo*);
    bdRemoteTaskRef findSessionsPaged(const unsigned int, bdSessionParams*, bdPagingToken*, bdMatchMakingInfo*);
    bdRemoteTaskRef findSessionsByEntityIDs(const unsigned long long*, const unsigned int, bdMatchMakingInfo*);
    bdRemoteTaskRef inviteToSession(bdSessionID*, unsigned long long*, const unsigned int, const void*, const unsigned int);
    bdRemoteTaskRef notifyJoin(bdSessionID*, const unsigned long long*, const unsigned int);
    bdRemoteTaskRef notifyLeave(bdSessionID*, const unsigned long long*, const unsigned int);
    bdRemoteTaskRef submitPerformance(const unsigned int, bdPerformanceValue*, const unsigned int);
    bdRemoteTaskRef getPerformanceValues(const unsigned long long*, const unsigned int, const unsigned int, bdPerformanceValue*);
    bdRemoteTaskRef getSessionInvites(const unsigned int, const unsigned int, bdSessionInvite*);
};