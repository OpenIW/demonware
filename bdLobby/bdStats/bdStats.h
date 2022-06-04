// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdStats
{
public:
    enum bdWriteType : bdInt
    {
        STAT_WRITE_REPLACE = 0,
        STAT_WRITE_ADD = 1,
        STAT_WRITE_MAX = 2,
        STAT_WRITE_MIN = 3,
        STAT_WRITE_REPLACE_WHEN_RATING_INCREASE = 4,
        STAT_WRITE_ADD_WHEN_RATING_INCREASE = 5,
        STAT_WRITE_MAX_WHEN_RATING_INCREASE = 6,
        STAT_WRITE_MIN_WHEN_RATING_INCREASE = 7
    };
protected:
    bdRemoteTaskManager* m_remoteTaskManager;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdStats(bdRemoteTaskManager* remoteTaskManager);
    ~bdStats();
    bdRemoteTaskRef writeStats(class bdStatsInfo** stats, const bdUInt numStats);
    bdRemoteTaskRef readStatsByRank(const unsigned int, const unsigned long long, class bdStatsInfo**, const unsigned int);
    bdRemoteTaskRef readStatsByRank(const bdUInt leaderBoardID, const bdUInt64 firstRank, bdStatsInfo* stats, const bdUInt maxResults);
    bdRemoteTaskRef readStatsByMultipleRanks(const unsigned int, const unsigned long long*, const unsigned int, class bdStatsInfo**);
    bdRemoteTaskRef readStatsByMultipleRanks(const unsigned int, const unsigned long long*, const unsigned int, class bdStatsInfo*);
    bdRemoteTaskRef readStatsByPivot(const unsigned int, const unsigned long long, class bdStatsInfo**, const unsigned int);
    bdRemoteTaskRef readStatsByPivot(const bdUInt leaderBoardID, const bdUInt64 entityID, bdStatsInfo* stats, const bdUInt maxResults);
    bdRemoteTaskRef readStatsByRating(const unsigned int, const long long, class bdStatsInfo**, const unsigned int);
    bdRemoteTaskRef readStatsByRating(const unsigned int, const long long, class bdStatsInfo*, const unsigned int);
    bdRemoteTaskRef readStatsByEntityID(const unsigned int, unsigned long long*, const unsigned int, class bdStatsInfo**);
    bdRemoteTaskRef readStatsByEntityID(const bdUInt leaderBoardID, bdUInt64* entityIDs, const bdUInt numEntityIDs, class bdStatsInfo* stats);
    bdRemoteTaskRef startArbitratedSession(const class bdSessionID, const class bdSessionNonce*);
    bdRemoteTaskRef writeArbitratedStats(const unsigned int, const class bdSessionID, const class bdSessionNonce*, class bdArbitratedStatsInfo**, const unsigned int);
    bdRemoteTaskRef deleteStats(const unsigned int);
    bdRemoteTaskRef deleteFileStats(const unsigned long long, const unsigned int);
    bdRemoteTaskRef deleteFileAndStats(const unsigned long long, const unsigned int, const unsigned int*);
};
