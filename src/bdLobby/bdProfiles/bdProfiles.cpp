// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdProfiles::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdProfiles::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdProfiles::bdProfiles(bdRemoteTaskManager* const remoteTaskManager)
    : m_remoteTaskManager(remoteTaskManager)
{
}

bdProfiles::~bdProfiles()
{
    m_remoteTaskManager = NULL;
}

bdRemoteTaskRef bdProfiles::deleteProfile()
{
    bdRemoteTaskRef task;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(64, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 8, 5);
    bdInt started = m_remoteTaskManager->startTask(task, buffer);
    if (started)
    {
        bdLogWarn("profiles", "Failed to start task: Error %i", started);
    }
    return task;
}

bdRemoteTaskRef bdProfiles::setPublicInfo(bdProfileInfo* profileInfo)
{
    bdRemoteTaskRef task;
    bdUInt taskSize = profileInfo->sizeOf() + 65;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 8, 3);
    profileInfo->serialize(**buffer);
    bdInt started = m_remoteTaskManager->startTask(task, buffer);
    if (started)
    {
        bdLogWarn("profiles", "Failed to start task: Error %i", started);
    }
    return task;
}

bdRemoteTaskRef bdProfiles::setPrivateInfo(bdProfileInfo* profileInfo)
{
    bdRemoteTaskRef task;
    bdUInt taskSize = profileInfo->sizeOf() + 65;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 8, 4);
    profileInfo->serialize(**buffer);
    bdInt started = m_remoteTaskManager->startTask(task, buffer);
    if (started)
    {
        bdLogWarn("profiles", "Failed to start task: Error %i", started);
    }
    return task;
}

bdRemoteTaskRef bdProfiles::getPublicInfos(const bdUInt64* userIDs, bdProfileInfo* publicProfiles, const bdUInt numProfiles)
{
    bdRemoteTaskRef task;
    bdUInt taskSize = 9 * numProfiles + 64;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 8, 1);
    bdBool ok = true;
    for (bdUInt i = 0; i < numProfiles && ok; ++i)
    {
        ok = ok == buffer->writeUInt64(userIDs[i]);
    }

    if (ok)
    {
        bdInt startTaskResult = m_remoteTaskManager->startTask(task, buffer);
        if (startTaskResult)
        {
            bdLogWarn("profiles", "Failed to start task: Error %i", startTaskResult);
        }
        else
        {
            task->setTaskResult(publicProfiles, numProfiles);
        }
    }
    else
    {
        bdLogWarn("profiles", "Failed to serialize the task buffer.");
    }
    return task;
}

bdRemoteTaskRef bdProfiles::getPrivateInfo(bdProfileInfo* const privateProfile)
{
    bdRemoteTaskRef task;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(64, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 8, 2);
    bdInt startTaskResult = m_remoteTaskManager->startTask(task, buffer);
    if (startTaskResult)
    {
        bdLogWarn("profiles", "Failed to start task: Error %i", startTaskResult);
    }
    else
    {
        task->setTaskResult(privateProfile, 1);
    }
    return task;
}
