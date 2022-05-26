// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdStats::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdStats::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdStats::bdStats(bdRemoteTaskManager* remoteTaskManager)
    : m_remoteTaskManager(remoteTaskManager)
{
}

bdRemoteTaskRef bdStats::writeStats(bdStatsInfo** stats, const bdUInt numStats)
{
    bdRemoteTaskRef task;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(0, true));
    for (bdUInt i = 0; i < numStats; ++i)
    {
        stats[i]->serialize(**buffer);
    }
    buffer->write(NULL, 64);
    buffer->allocateBuffer();
    m_remoteTaskManager->initTaskBuffer(buffer, 4, 1);
    for (bdUInt i = 0; i < numStats; ++i)
    {
        stats[i]->serialize(**buffer);
    }
    bdInt startTaskResult = m_remoteTaskManager->startTask(task, buffer);
    if (startTaskResult)
    {
        bdLogWarn("stats", "Failed to start task: Error %i", startTaskResult);
    }
    return task;
}

bdRemoteTaskRef bdStats::readStatsByRank(const bdUInt leaderBoardID, const bdUInt64 firstRank, bdStatsInfo* stats, const bdUInt maxResults)
{
    bdRemoteTaskRef task;
    bdUInt taskSize = 83;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 4, 4);
    bdBool ok = buffer->writeUInt32(leaderBoardID);
    ok = ok == buffer->writeUInt64(firstRank);
    ok = ok == buffer->writeUInt32(maxResults);
    if (ok)
    {
        bdInt startTaskResult = m_remoteTaskManager->startTask(task, buffer);
        if (startTaskResult)
        {
            bdLogWarn("stats", "Failed to start task: Error %i", startTaskResult);
        }
        else
        {
            task->setTaskResult(stats, maxResults);
        }
    }
    else
    {
        bdLogWarn("stats", "Failed to write param into buffer");
    }
    return task;
}

bdRemoteTaskRef bdStats::readStatsByPivot(const bdUInt leaderBoardID, const bdUInt64 entityID, bdStatsInfo* stats, const bdUInt maxResults)
{
    bdRemoteTaskRef task;
    bdUInt taskSize = 83;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 4, 5);
    bdBool ok = buffer->writeUInt32(leaderBoardID);
    ok = ok == buffer->writeUInt64(entityID);
    ok = ok == buffer->writeUInt32(maxResults);
    if (ok)
    {
        bdInt startTaskResult = m_remoteTaskManager->startTask(task, buffer);
        if (startTaskResult)
        {
            bdLogWarn("stats", "Failed to start task: Error %i", startTaskResult);
        }
        else
        {
            task->setTaskResult(stats, maxResults);
        }
    }
    else
    {
        bdLogWarn("stats", "Failed to write param into buffer");
    }
    return task;
}

bdRemoteTaskRef bdStats::readStatsByEntityID(const bdUInt leaderBoardID, bdUInt64* entityIDs, const bdUInt numEntityIDs, bdStatsInfo* stats)
{
    bdRemoteTaskRef task;
    bdUInt taskSize = 9 * numEntityIDs + 69;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 4, 3);
    bdBool ok = buffer->writeUInt32(leaderBoardID);
    for (bdUInt i = 0; i < numEntityIDs; ++i)
    {
        ok = ok == buffer->writeUInt64(entityIDs[i]);
    }

    if (ok)
    {
        bdInt startTaskResult = m_remoteTaskManager->startTask(task, buffer);
        if (startTaskResult)
        {
            bdLogWarn("stats", "Failed to start task: Error %i", startTaskResult);
        }
        else
        {
            task->setTaskResult(stats, numEntityIDs);
        }
    }
    else
    {
        bdLogWarn("stats", "Failed to write param into buffer");
    }
    return task;
}
