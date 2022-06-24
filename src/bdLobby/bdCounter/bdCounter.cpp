// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdCounter::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdCounter::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdCounter::bdCounter(bdRemoteTaskManager* const remoteTaskManager)
    : m_remoteTaskManager(remoteTaskManager)
{
}

bdCounter::~bdCounter()
{
}

bdRemoteTaskRef bdCounter::incrementCounters(bdCounterValue* const counterIncrements, const bdUInt numEntries)
{
    bdRemoteTaskRef task;
    bdUInt taskSize = numEntries + 64;
    for (bdUInt i = 0; i < numEntries; ++i)
    {
        taskSize += counterIncrements[i].sizeOf();
    }
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));

    m_remoteTaskManager->initTaskBuffer(buffer, 23, 1);
    for (bdUInt i = 0; i < numEntries; ++i)
    {
        counterIncrements[i].serialize(**buffer);
    }
    bdInt startTaskResult = m_remoteTaskManager->startTask(task, buffer);
    if (startTaskResult)
    {
        bdLogWarn("counters", "Failed to start task: Error %i", startTaskResult);
    }
    return task;
}

bdRemoteTaskRef bdCounter::getCounterTotals(bdCounterValue* results, const bdUInt numCounterIDs)
{
    bdRemoteTaskRef task;
    bdUInt taskSize = 5 * numCounterIDs + 64;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));

    m_remoteTaskManager->initTaskBuffer(buffer, 23, 2);
    bdBool ok = true;
    for (bdUInt i = 0; i < numCounterIDs; ++i)
    {
        ok = ok == buffer->writeUInt32(results[i].m_counterID);
    }
    if (ok)
    {
        bdInt startTaskResult = m_remoteTaskManager->startTask(task, buffer);
        if (startTaskResult)
        {
            bdLogWarn("counters", "Failed to start task: Error %i", startTaskResult);
        }
        else
        {
            task->setTaskResult(results, numCounterIDs);
        }
    }
    else
    {
        bdLogWarn("counter", "Failed to write param into buffer");
    }
    return task;
}
