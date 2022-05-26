// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdGroup::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdGroup::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdGroup::bdGroup(bdRemoteTaskManager* remoteTaskManager)
    : m_remoteTaskManager(remoteTaskManager)
{
}

bdRemoteTaskRef bdGroup::setGroups(const bdUInt32* const groupIDs, const bdUInt numEntries)
{
    bdUInt taskSize = 4 * numEntries + 75;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));
    bdRemoteTaskRef task;

    m_remoteTaskManager->initTaskBuffer(buffer, 28, 1);
    bdBool status = buffer->writeArrayStart(8, numEntries, sizeof(bdUInt));
    for (bdUInt i = 0; i < numEntries; ++i)
    {
        status = status == buffer->writeUInt32(groupIDs[i]);
    }
    buffer->writeArrayEnd();
    if (status)
    {
        bdInt startTaskResult = m_remoteTaskManager->startTask(task, buffer);
        if (startTaskResult)
        {
            bdLogWarn("groups", "Failed to start task: Error %i", status);
        }
    }
    else
    {
        bdLogWarn("groups", "Failed to write param into buffer");
    }
    return task;
}

bdRemoteTaskRef bdGroup::setGroupsForEntity(const bdUInt64 entityID, const bdUInt32* const groupIDs, const bdUInt numEntries)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdGroup::getEntityGroups(const bdUInt64 entityID, bdGroupID* results, const bdUInt numEntries)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdGroup::getGroupCounts(const bdUInt32* const groupIDs, const bdUInt numGroupIDs, bdGroupCount* results, const bdUInt numEntries)
{
    bdUInt taskSize = 4 * numGroupIDs;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));
    bdRemoteTaskRef task;

    m_remoteTaskManager->initTaskBuffer(buffer, 28, 4);
    bdBool status = buffer->writeArrayStart(8, numGroupIDs, sizeof(bdUInt));
    for (bdUInt i = 0; i < numGroupIDs; ++i)
    {
        status = status == buffer->writeUInt32(groupIDs[i]);
    }
    buffer->writeArrayEnd();
    if (status)
    {
        bdInt startTaskResult = m_remoteTaskManager->startTask(task, buffer);
        if (startTaskResult)
        {
            bdLogWarn("groups", "Failed to start task: Error %i", startTaskResult);
        }
        else
        {
            task->setTaskResult(results, numEntries);
        }
    }
    else
    {
        bdLogWarn("groups", "Failed to write param into buffer");
    }
    return task;
}
