// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdMatchMaking::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdMatchMaking::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);;
}

bdMatchMaking::bdMatchMaking(bdRemoteTaskManager* remoteTaskManager)
    : m_remoteTaskManager(remoteTaskManager)
{
}

bdMatchMaking::~bdMatchMaking()
{
}

bdRemoteTaskRef bdMatchMaking::createSession(bdMatchMakingInfo* sessionInfo, bdSessionID* sessionCredentials)
{
    bdRemoteTaskRef task;
    bdUInt taskSize = sessionInfo->sizeOf() + 65;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 21, 1);
    sessionInfo->serialize(**buffer);
    bdInt startTaskResult = m_remoteTaskManager->startTask(task, buffer);
    if (startTaskResult)
    {
        bdLogWarn("matchmaking", "Failed to start task: Error %i", startTaskResult);
    }
    else
    {
        task->setTaskResult(sessionCredentials, 1);
    }
    return task;
}

bdRemoteTaskRef bdMatchMaking::updateSession(const bdSessionID& sessionID, bdMatchMakingInfo* const sessionInfo)
{
    bdRemoteTaskRef task;
    bdUInt taskSize = sessionInfo->sizeOf() + 78;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 21, 2);
    sessionID.serialize(**buffer);
    sessionInfo->serialize(**buffer);
    bdInt startTaskResult = m_remoteTaskManager->startTask(task, buffer);
    if (startTaskResult)
    {
        bdLogWarn("matchmaking", "Failed to start task: Error %i", startTaskResult);
    }
    return task;
}

bdRemoteTaskRef bdMatchMaking::updateSessionPlayers(const bdSessionID& sessionID, bdMatchMakingInfo* const sessionInfo, bdUInt32 numPlayers)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdMatchMaking::deleteSession(const bdSessionID& sessionID)
{
    bdRemoteTaskRef task;
    bdUInt taskSize = 77;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 21, 3);
    sessionID.serialize(**buffer);
    bdInt startTaskResult = m_remoteTaskManager->startTask(task, buffer);
    if (startTaskResult)
    {
        bdLogWarn("matchmaking", "Failed to start task: Error %i", startTaskResult);
    }
    return task;
}

bdRemoteTaskRef bdMatchMaking::findSessionFromID(const bdSessionID& sessionID, bdMatchMakingInfo* const sessionInfo)
{
    bdRemoteTaskRef task;
    bdUInt taskSize = 77;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 21, 4);
    sessionID.serialize(**buffer);
    bdInt startTaskResult = m_remoteTaskManager->startTask(task, buffer);
    if (startTaskResult)
    {
        bdLogWarn("matchmaking", "Failed to start task: Error %i", startTaskResult);
    }
    else
    {
        task->setTaskResult(sessionInfo, 1);
    }
    return task;
}

bdRemoteTaskRef bdMatchMaking::findSessions(const bdUInt queryID, const bdUInt startIndexDEPRECATED, const bdUInt maxNumResults, bdSessionParams* sessionParams, bdMatchMakingInfo* results)
{
    bdRemoteTaskRef task;
    bdUInt taskSize = sessionParams->sizeOf() + 80;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 21, 5);
    bdBool ok = buffer->writeUInt32(queryID);
    ok = ok == buffer->writeUInt32(startIndexDEPRECATED);
    ok = ok == buffer->writeUInt32(maxNumResults);
    sessionParams->serialize(**buffer);
    if (ok)
    {
        bdInt startTaskResult = m_remoteTaskManager->startTask(task, buffer);
        if (startTaskResult)
        {
            bdLogWarn("matchmaking", "Failed to start task: Error %i", startTaskResult);
        }
        else
        {
            task->setTaskResult(results, maxNumResults);
        }
    }
    else
    {
        bdLogWarn("matchmaking", "Failed to write param into buffer");
    }
    return task;
}

bdRemoteTaskRef bdMatchMaking::findSessionsPaged(const bdUInt queryID, bdSessionParams* sessionParams, bdPagingToken* token, bdMatchMakingInfo* results)
{
    bdRemoteTaskRef task;
    bdUInt taskSize = sessionParams->sizeOf() + 90;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 21, 13);
    bdPagingToken::bdStatus tokenStatus = token->getStatus();
    if (tokenStatus == bdPagingToken::BD_FINISHED)
    {
        bdLogWarn("matchmaking", "Paging token is invalid.");
    }
    else
    {
        const bdSecurityID sessionID = token->getSessionID();
        bdBool ok = buffer->writeUInt32(queryID);
        ok = ok == buffer->writeBool(tokenStatus == bdPagingToken::BD_NOT_STARTED);
        ok = ok == buffer->writeBlob(sessionID.ab, sizeof(bdSecurityID));
        ok = ok == buffer->writeUInt32(token->getNumResultsPerPage());
        sessionParams->serialize(**buffer);
        if (ok)
        {
            bdInt startTaskResult = m_remoteTaskManager->startTask(task, buffer);
            if (startTaskResult)
            {
                bdLogWarn("matchmaking", "Failed to start task: Error %i", startTaskResult);
            }
            else
            {
                task->setTaskResult(results, token->getNumResultsPerPage());
                task->setTaskResultProcessor(token);
            }
        }
        else
        {
            bdLogWarn("matchmaking", "Failed to write param into buffer");
        }
    }
    return task;
}

bdRemoteTaskRef bdMatchMaking::findSessionsByEntityIDs(const bdUInt64* const entityIDs, const bdUInt numEntityIDs, bdMatchMakingInfo* results)
{
    bdRemoteTaskRef task;
    bdUInt taskSize = 9 * numEntityIDs + 64;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 21, 14);
    bdBool ok = true;
    for (bdUInt i = 0; i < numEntityIDs; ++i)
    {
        ok = ok == buffer->writeUInt64(entityIDs[i]);
    }
    if (ok)
    {
        bdInt startTaskResult = m_remoteTaskManager->startTask(task, buffer);
        if (startTaskResult)
        {
            bdLogWarn("matchmaking", "Failed to start task: Error %i", startTaskResult);
        }
        else
        {
            task->setTaskResult(results, numEntityIDs);
        }
    }
    else
    {
        bdLogWarn("matchmaking", "Failed to write param into buffer");
    }
    return task;
}

bdRemoteTaskRef bdMatchMaking::inviteToSession(const bdSessionID& sessionID, const bdUInt64* userIDs, const bdUInt numUsers, const void* const attachment, const bdUInt attachmentSize)
{
    bdRemoteTaskRef task;
    bdUInt clampedAttachmentSize = attachmentSize >= 0x400 ? 0x400 : attachmentSize;
    if (clampedAttachmentSize < attachmentSize)
    {
        bdLogError("matchmaking", "Exceeded maximum invite attachment size, data will be truncated.");
    }
    bdUInt taskSize = clampedAttachmentSize + (9 * numUsers) + 81;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 21, 8);
    sessionID.serialize(**buffer);
    bdBool ok = buffer->writeBlob(attachment, clampedAttachmentSize);
    for (bdUInt i = 0; i < numUsers; ++i)
    {
        ok = ok == buffer->writeUInt64(userIDs[i]);
    }
    if (ok)
    {
        bdInt startTaskResult = m_remoteTaskManager->startTask(task, buffer);
        if (startTaskResult)
        {
            bdLogWarn("matchmaking", "Failed to start task: Error %i", startTaskResult);
        }
    }
    else
    {
        bdLogWarn("matchmaking", "Failed to write param into buffer");
    }
    return task;
}

bdRemoteTaskRef bdMatchMaking::notifyJoin(const bdSessionID& sessionID, const bdUInt64* users, const bdUInt numUsers)
{
    return startNotifyTask(sessionID, users, numUsers, 6);
}

bdRemoteTaskRef bdMatchMaking::notifyLeave(const bdSessionID& sessionID, const bdUInt64* users, const bdUInt numUsers)
{
    return startNotifyTask(sessionID, users, numUsers, 7);
}

bdRemoteTaskRef bdMatchMaking::submitPerformance(const bdUInt gameType, bdPerformanceValue* performanceValues, const bdUInt numValues)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdMatchMaking::getPerformanceValues(const bdUInt64* const entityIDs, const bdUInt numEntityIDs, const bdUInt gameType, bdPerformanceValue* results)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdMatchMaking::getSessionInvites(const bdUInt32 startIndex, const bdUInt32 maxNumResults, bdSessionInvite* results)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdMatchMaking::startNotifyTask(const bdSessionID& sessionID, const bdUInt64* users, const bdUInt numUsers, const bdUByte8 taskType)
{
    bdRemoteTaskRef task;
    bdUInt taskSize = 9 * numUsers + 77;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 21, taskType);
    sessionID.serialize(**buffer);
    bdBool ok = true;
    for (bdUInt i = 0; i < numUsers; ++i)
    {
        ok = ok == buffer->writeUInt64(users[i]);
    }
    if (ok)
    {
        bdInt startTaskResult = m_remoteTaskManager->startTask(task, buffer);
        if (startTaskResult)
        {
            bdLogWarn("matchmaking", "Failed to start task: Error %i", startTaskResult);
        }
    }
    else
    {
        bdLogWarn("matchmaking", "Failed to write param into buffer");
    }
    return task;
}
