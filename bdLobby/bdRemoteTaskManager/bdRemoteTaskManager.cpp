// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdRemoteTaskManager::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdRemoteTaskManager::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdRemoteTaskManager::bdRemoteTaskManager(bdLobbyConnectionRef lobbyConnection, const bdBool useEncryption)
    : m_tasks(), m_asyncTasks(4u, 0.75f), m_asyncResults(4u, 0.75f), m_lobbyConnection(*m_lobbyConnection), m_encryptedConnection(useEncryption), m_connectionID(0)
{
}

bdRemoteTaskManager::~bdRemoteTaskManager()
{
}

void bdRemoteTaskManager::initTaskBuffer(bdTaskByteBufferRef* buffer, const bdUByte8 serviceID, const bdUByte8 taskID)
{
    (*buffer)->write(&serviceID, sizeof(serviceID));
    (*buffer)->writeUByte8(taskID);
}

bdLobbyErrorCode bdRemoteTaskManager::startTask(bdRemoteTaskRef* newTask, bdTaskByteBufferRef* queryParams)
{
    *newTask = new bdRemoteTask();
    return sendTask(&bdRemoteTaskRef(newTask), queryParams);
}

bdLobbyErrorCode bdRemoteTaskManager::startLSGTask(bdRemoteTaskRef* newTask, const bdUByte8 serviceID, const bdUByte8 taskID, const void* const queryParams, const bdUInt queryParamsSize)
{
    return bdLobbyErrorCode();
}

bdLobbyErrorCode bdRemoteTaskManager::sendTask(bdRemoteTaskRef newTask, bdTaskByteBufferRef* queryParams)
{
    bdBool ok = false;
    ok = (*queryParams)->writeNoType();
    ok = ok == m_lobbyConnection->sendTask(&bdTaskByteBufferRef(queryParams), (*queryParams)->getDataSize(), m_encryptedConnection);
    if (!ok)
    {
        newTask->setStatus(bdRemoteTask::BD_FAILED);
        newTask->setErrorCode(BD_SEND_FAILED);
        return BD_SEND_FAILED;
    }
    m_tasks.addTail(&newTask);
    newTask->start(0.0f);
    return BD_NO_ERROR;
}

bdUInt64 bdRemoteTaskManager::getConnectionID() const
{
    return m_connectionID;
}

void bdRemoteTaskManager::handleTaskReply(const bdByteBufferRef buffer)
{
    bdRemoteTaskRef task(m_tasks.getHead());
    if (task.notNull() && task->getStatus() == bdRemoteTask::BD_PENDING)
    {
        bdUInt64 transactionID = 0;
        if ((*buffer)->read<bdUInt64>(&transactionID))
        {
            task->stop(&bdByteBufferRef(*buffer), transactionID);
            bdLogInfo("lobby/remotetaskmanager", "Received reply on\nconnection ID : %llu\ntransaction ID: %llu", m_connectionID, transactionID);
        }
        else
        {
            bdLogWarn("lobby/remotetaskmanager", "Failed to retrieve transaction ID");
        }
    }
    m_tasks.removeHead();
}

void bdRemoteTaskManager::handleLSGTaskReply(const bdByteBufferRef buffer)
{
    bdRemoteTaskRef task(m_tasks.isEmpty() ? NULL : m_tasks.getHead());
    if (task.notNull() && task->getStatus() == bdRemoteTask::BD_PENDING)
    {
        task->handleTaskReply(&bdByteBufferRef(*buffer));
        if (task->getStatus() == bdRemoteTask::BD_PENDING)
        {
            bdByteBufferRef asyncResult;
            bdUInt64 key = task->getTransactionID();
            if (m_asyncResults.remove(&key, &asyncResult))
            {
                task->handleAsyncTaskReply(&bdByteBufferRef(&asyncResult));
            }
            else
            {
                m_asyncTasks.put(&key, &task);
            }
        }
    }
    m_tasks.removeHead();
}

void bdRemoteTaskManager::handleAsyncResult(const bdByteBufferRef buffer)
{
    bdUInt64 transactionID = 0;
    bdRemoteTaskRef task;
    if ((*buffer)->readUInt64(&transactionID))
    {
        if (m_asyncTasks.get(&transactionID, &task))
        {
            bdRemoteTask::bdStatus status = task->getStatus();
            if (status == bdRemoteTask::BD_PENDING)
            {
                task->handleAsyncTaskReply(&bdByteBufferRef(*buffer));
                if (task->getStatus() != bdRemoteTask::BD_PENDING)
                {
                    m_asyncTasks.remove(&transactionID);
                }
            }
            else
            {
                bdLogInfo("lobby/remotetaskmanager", "Discarding asynchronous result for bdRemoteTask that is no longer in BD_PENDING (status: %i).", status);
            }
        }
        else if (m_tasks.getSize())
        {
            bdLogWarn("lobby/remotetaskmanager", "Received asynchronous result for task that might not be complete yet.  Storing for later (transaction ID: %llu)", transactionID);
            m_asyncResults.put(&transactionID, &bdByteBufferRef(*buffer));
        }
        else
        {
            bdLogWarn("lobby/remotetaskmanager", "Received asynchronous result for unknown task (transactionID: %llu)", transactionID);
        }
    }
    else
    {
        bdLogError("lobby/remotetaskmanager", "Failed to deserialize asynchronous task result!");
    }
}

void bdRemoteTaskManager::cleanUpAsyncState()
{
    bdHashMap<bdUInt64,bdReference<bdRemoteTask>,bdHashingClass>::Iterator i = m_asyncTasks.getIterator();
    for (i = m_asyncTasks.getIterator(); i; m_asyncTasks.next(&i))
    {
        if ((*m_asyncTasks.getValue(i))->getStatus() == bdRemoteTask::BD_PENDING)
        {
            continue;
        }
        bdLogInfo("lobby/remotetaskmanager", "Removing asynchronous task that is no longer BD_PENDING (transaction ID: %llu).", *m_asyncTasks.getKey(i));
        m_asyncTasks.remove(&i);
    }
    if (m_tasks.getSize() && m_asyncTasks.getSize() && m_asyncResults.getSize() > 0)
    {
        for (i = m_asyncResults.getIterator(); i; m_asyncResults.next(&i))
        {
            bdLogInfo("lobby/remotetaskmanager", "Detected orphaned asynchronous result (transaction ID: %llu).  Discarding.", *m_asyncResults.getKey(i));
        }
        m_asyncResults.clear();
    }
}
