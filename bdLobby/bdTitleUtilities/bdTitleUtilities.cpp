// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdTitleUtilities::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdTitleUtilities::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdTitleUtilities::bdTitleUtilities(bdRemoteTaskManager* remoteTaskManager)
    : m_remoteTaskManager(remoteTaskManager)
{
}

bdRemoteTaskRef bdTitleUtilities::verifyString(const bdNChar8* const str, const bdUInt length, bdVerifyString* verified)
{
    bdRemoteTaskRef task;
    bdUInt strLen;
    if (str)
    {
        strLen = bdStrnlen(str, length) + 2;
    }
    else
    {
        strLen = 0;
    }
    bdUInt taskSize = strLen + 64;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 12, 1);
    bdBool ok = buffer->writeString(str, length);
    if (ok)
    {
        bdInt startTaskResult = m_remoteTaskManager->startTask(task, buffer);
        if (startTaskResult)
        {
            bdLogWarn("title utilities", "Failed to start task: Error %i", startTaskResult);
        }
        else
        {
            task->setTaskResult(verified, 1);
        }
    }
    else
    {
        bdLogWarn("title utilities", "Failed to write string %s into buffer", str);
    }
    return task;
}

bdRemoteTaskRef bdTitleUtilities::recordEvent(const bdNChar8* const eventStr, const bdUInt length, bdUInt category)
{
    bdRemoteTaskRef task;
    bdUInt strLen;
    if (eventStr)
    {
        strLen = bdStrnlen(eventStr, length) + 2;
    }
    else
    {
        strLen = 0;
    }
    bdUInt taskSize = strLen + 69;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 12, 3);
    bdBool ok = buffer->writeString(eventStr, length);
    buffer->writeUInt32(category);
    if (ok)
    {
        bdInt startTaskResult = m_remoteTaskManager->startTask(task, buffer);
        if (startTaskResult)
        {
            bdLogWarn("title utilities", "Failed to start task: Error %i", startTaskResult);
        }
    }
    else
    {
        bdLogWarn("title utilities", "Failed to write string %s into buffer", eventStr);
    }
    return task;
}

bdRemoteTaskRef bdTitleUtilities::recordEventBin(const void* eventBlob, const bdUInt length, bdUInt category)
{
    bdRemoteTaskRef task;
    bdUInt taskSize = length + 74;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 12, 5);
    buffer->writeBlob(eventBlob, length);
    buffer->writeUInt32(category);
    m_remoteTaskManager->startTask(task, buffer);
    return task;
}

bdRemoteTaskRef bdTitleUtilities::getServerTime(bdTimeStamp* timeStamp)
{
    bdRemoteTaskRef task;
    bdUInt taskSize = 64;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 12, 6);
    bdInt startTaskResult = m_remoteTaskManager->startTask(task, buffer);
    if (startTaskResult)
    {
        bdLogWarn("title utilities", "Failed to start task: Error %i", startTaskResult);
    }
    else
    {
        task->setTaskResult(timeStamp, 1);
    }
    return task;
}
