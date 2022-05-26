// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdStorage::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdStorage::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdStorage::bdStorage(bdRemoteTaskManager* remoteTaskManager)
    : m_remoteTaskManager(remoteTaskManager)
{
}

bdStorage::~bdStorage()
{
}

bdRemoteTaskRef bdStorage::uploadFile(const bdNChar8* const fileName, const void* const fileData, const bdUInt fileSize, const bdFileInfo::bdVisibility fileVisibility,
    const bdUInt64 ownerID, bdFileInfo* const fileInfo)
{
    bdRemoteTaskRef task;
    bdUInt fileNameLen;
    if (fileName)
    {
        fileNameLen = bdStrnlen(fileName, 128) + 2;
    }
    else
    {
        fileNameLen = 0;
    }
    bdUInt taskSize = fileNameLen + fileSize + 71;
    if (ownerID)
    {
        taskSize += 9;
    }
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 10, 1);
    bdBool ok = buffer->writeString(fileName, 128);
    ok = ok == buffer->writeBool(fileVisibility == 1);
    ok = ok == buffer->writeBlob(fileData, fileSize);
    if (ownerID)
    {
        ok = ok == buffer->writeUInt64(ownerID);
    }
    if (ok)
    {
        bdInt startTaskResult = m_remoteTaskManager->startTask(task, buffer);
        if (startTaskResult)
        {
            bdLogWarn("storage", "Failed to start task: Error %i", startTaskResult);
        }
        else if (fileInfo)
        {
            task->setTaskResult(fileInfo, 1);
        }
    }
    else
    {
        bdLogWarn("storage", "Failed to write param into buffer");
    }
    return &task;
}

bdRemoteTaskRef bdStorage::removeFile(const bdNChar8* const fileName, const bdUInt64 ownerID)
{
    bdRemoteTaskRef task;
    bdUInt fileNameLen;
    if (fileName)
    {
        fileNameLen = bdStrnlen(fileName, 128) + 2;
    }
    else
    {
        fileNameLen = 0;
    }
    bdUInt taskSize = fileNameLen + 64;
    if (ownerID)
    {
        taskSize += 9;
    }
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 10, 2);
    bdBool ok = buffer->writeString(fileName, 128);
    if (ownerID)
    {
        ok = ok == buffer->writeUInt64(ownerID);
    }
    if (ok)
    {
        bdInt startTaskResult = m_remoteTaskManager->startTask(task, buffer);
        if (startTaskResult)
        {
            bdLogWarn("storage", "Failed to start task: Error %i", startTaskResult);
        }
    }
    else
    {
        bdLogWarn("storage", "Failed to write param into buffer");
    }
    return task;
}

bdRemoteTaskRef bdStorage::getFile(const bdNChar8* const fileName, bdFileData* const fileData, const bdUInt64 ownerUID)
{
    bdRemoteTaskRef task;
    bdUInt fileNameLen;
    if (fileName)
    {
        fileNameLen = bdStrnlen(fileName, 128) + 2;
    }
    else
    {
        fileNameLen = 0;
    }
    bdUInt taskSize = fileNameLen + 73;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 10, 3);
    bdBool ok = buffer->writeString(fileName, 128);
    ok = ok == buffer->writeUInt64(ownerUID);
    if (ok)
    {
        bdInt startTaskResult = m_remoteTaskManager->startTask(task, buffer);
        if (startTaskResult)
        {
            bdLogWarn("storage", "Failed to start task: Error %i", startTaskResult);
        }
        else
        {
            task->setTaskResult(fileData, 1);
        }
    }
    else
    {
        bdLogWarn("storage", "Failed to write param into buffer");
    }
    return task;
}

bdRemoteTaskRef bdStorage::getPublisherFile(const bdNChar8* const fileName, bdFileData* const fileData)
{
    bdRemoteTaskRef task;
    bdUInt fileNameLen;
    if (fileName)
    {
        fileNameLen = bdStrnlen(fileName, 128) + 2;
    }
    else
    {
        fileNameLen = 0;
    }
    bdUInt taskSize = fileNameLen + 64;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 10, 7);
    bdBool ok = buffer->writeString(fileName, 128);
    if (ok)
    {
        bdInt startTaskResult = m_remoteTaskManager->startTask(task, buffer);
        if (startTaskResult)
        {
            bdLogWarn("storage", "Failed to start task: Error %i", startTaskResult);
        }
        else
        {
            task->setTaskResult(fileData, 1);
        }
    }
    else
    {
        bdLogWarn("storage", "Failed to write param into buffer");
    }
    return task;
}
