// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdLobby/bdLobby.h"

void bdRemoteTask::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdRemoteTask::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdRemoteTask::bdRemoteTask()
    : bdReferencable(), m_timer(), m_timeout(0.0f), m_status(BD_EMPTY), m_byteResults(NULL), m_taskResult(NULL), m_taskResultList(NULL), m_numResults(0),
    m_maxNumResults(0), m_totalNumResults(0), m_transactionID(0), m_errorCode(BD_NO_ERROR), m_taskResultProcessor(NULL)
{
    m_timer.reset();
}

bdRemoteTask::bdRemoteTask(bdRemoteTask::bdStatus status)
    : bdReferencable(), m_timer(), m_timeout(0.0f), m_status(status), m_byteResults(NULL), m_taskResult(NULL), m_taskResultList(NULL), m_numResults(0),
    m_maxNumResults(0), m_totalNumResults(0), m_transactionID(0), m_errorCode(BD_NO_ERROR), m_taskResultProcessor(NULL)
{
}

bdRemoteTask::~bdRemoteTask()
{
}

void bdRemoteTask::cancelTask()
{
    m_status = BD_CANCELLED;
}

void bdRemoteTask::handleTaskReply(bdByteBufferRef buffer)
{
    bdUByte8 taskId;

    bdUInt32 error = 4;
    bdBool ok = buffer->readUInt64(m_transactionID);
    if (ok)
    {
        bdLogInfo("remote task", "Received task reply (transaction ID: %llu).", m_transactionID);
        ok = buffer->readUInt32(error);
    }
    if (ok)
    {
        if (error)
        {
            if (error != BD_ASYNCHRONOUS_ERROR)
            {
                m_errorCode = static_cast<bdLobbyErrorCode>(error);
                m_status = BD_FAILED;
                bdLogWarn("remote task", "Remote task failed (transaction ID: %llu, error code: %u).", m_transactionID, m_errorCode);
            }
        }
        else
        {
            m_errorCode = BD_NO_ERROR;
            ok = ok == buffer->readUByte8(taskId);
            ok = ok == deserializeTaskReply(bdByteBufferRef(buffer));
            m_status = BD_DONE;
        }
    }
    if (!ok)
    {
        m_errorCode = BD_HANDLE_TASK_FAILED;
        m_status = BD_FAILED;
        bdLogError("remote task", "Failed to deserialize task result!");
    }
}

void bdRemoteTask::handleAsyncTaskReply(bdByteBufferRef buffer)
{
    bdUInt32 error = 4;
    bdLogInfo("remote task", "Received asynchronous task reply (transaction ID: %llu).", m_transactionID);
    bdBool ok = buffer->readUInt32(error);
    if (ok)
    {
        if (error)
        {
            if (error == BD_ASYNCHRONOUS_ERROR)
            {
                m_errorCode = BD_HANDLE_TASK_FAILED;
                m_status = BD_FAILED;
                bdLogError("remote task", "Received BD_ASYNCHRONOUS_ERROR in asynchronous reply (transaction ID: %llu).  This should never happen!", m_transactionID);
            }
            else
            {
                m_errorCode = static_cast<bdLobbyErrorCode>(error);
                m_status = BD_FAILED;
                bdLogWarn("remote task", "Remote task failed (transaction ID: %llu, error code: %u).", m_transactionID, m_errorCode);
            }
        }
        else
        {
            m_errorCode = BD_NO_ERROR;
            ok = ok == deserializeTaskReply(bdByteBufferRef(buffer));
            m_status = BD_DONE;
        }
    }
    if (!ok)
    {
        m_errorCode = BD_HANDLE_TASK_FAILED;
        m_status = BD_FAILED;
        bdLogError("remote task", "Failed to deserialize asynchronous task result (transaction ID: %llu)!", m_transactionID);
    }
}

bdBool bdRemoteTask::deserializeTaskReply(bdByteBufferRef buffer)
{
    bdBool ok = true;

    ok = ok == buffer->readUInt32(m_numResults);
    if (ok)
    {
        if (m_numResults)
        {
            ok = ok == buffer->readUInt32(m_totalNumResults);
            if (ok)
            {
                if (m_taskResult)
                {
                    bdUInt taskResultSize = m_taskResult->sizeOf();
                    bdUInt numResultsToRead = m_numResults >= m_maxNumResults ? m_maxNumResults : m_numResults;
                    bdTaskResult* taskResult = m_taskResult;
                    for (bdUInt i = 0; i < numResultsToRead; ++i)
                    {
                        taskResult->deserialize(bdByteBufferRef(buffer));
                        taskResult = (bdTaskResult*)((char*)taskResult + taskResultSize);
                    }
                    if (m_taskResultProcessor)
                    {
                        ok = ok == m_taskResultProcessor->processResult(m_taskResult, numResultsToRead);
                    }
                }
                else if (m_taskResultList)
                {
                    for (bdUInt i = 0; i < (m_numResults >= m_maxNumResults ? m_maxNumResults : m_numResults) && ok; ++i)
                    {
                        ok = ok == m_taskResultList[i]->deserialize(bdByteBufferRef(buffer));
                    }
                }
                else
                {
                    bdLogWarn("remote task", "Received task result but have nowhere to deserialize to (transaction ID: %llu).", m_transactionID);
                }
            }
        }
        else if (m_taskResultProcessor)
        {
            ok = ok == m_taskResultProcessor->processResult(m_taskResult, 0);
        }
    }
    if (!ok)
    {
        bdLogError("remote task", "Failed to deserialize task results (transaction ID: %I64u)!", m_transactionID);
    }
    return ok;
}

void bdRemoteTask::start(const bdFloat32 timeout)
{
    m_timer.start();
    m_timeout = timeout;
    m_status = BD_PENDING;
    m_byteResults = NULL;
}

void bdRemoteTask::stop(bdByteBufferRef results, const bdUInt64 id)
{
    m_status = BD_DONE;
    m_byteResults = results;
    m_transactionID = id;
}

void bdRemoteTask::setNumResults(bdUInt numResults)
{
    m_numResults = numResults;
}

void bdRemoteTask::setStatus(const bdRemoteTask::bdStatus status)
{
    m_status = status;
}

void bdRemoteTask::setErrorCode(const bdLobbyErrorCode errorCode)
{
    m_errorCode = errorCode;
}

void bdRemoteTask::setTimeout(const bdFloat32 timeout)
{
    m_timeout = timeout;
}

void bdRemoteTask::setTaskResult(bdTaskResult* result, const bdUInt numResults)
{
    m_taskResult = result;
    m_maxNumResults = numResults;
}

void bdRemoteTask::setTaskResultList(bdTaskResult** result, const bdUInt numResults)
{
    m_taskResultList = result;
    m_maxNumResults = numResults;
}

void bdRemoteTask::setTaskResultProcessor(bdTaskResultProcessor* processor)
{
    m_taskResultProcessor = processor;
}

bdRemoteTask::bdStatus bdRemoteTask::getStatus()
{
    if (m_status == BD_PENDING && m_timeout > 0.0)
    {
        if (m_timeout <= m_timer.getElapsedTimeInSeconds())
        {
            m_status = BD_TIMED_OUT;
            m_byteResults = (bdByteBuffer*)NULL;
            bdLogInfo("remote task", "Remote task timed out after %.3fs.", m_timeout);
        }
    }
    return m_status;
}

bdUInt bdRemoteTask::getNumResults() const
{
    return m_numResults;
}

bdUInt bdRemoteTask::getTotalNumResults() const
{
    return m_totalNumResults;
}

bdByteBufferRef bdRemoteTask::getByteResults() const
{
    return m_byteResults;
}

bdStopwatch bdRemoteTask::getTimer() const
{
    return m_timer;
}

bdFloat32 bdRemoteTask::getTimeout() const
{
    return m_timeout;
}

bdUInt64 bdRemoteTask::getTransactionID() const
{
    return m_transactionID;
}

bdLobbyErrorCode bdRemoteTask::getErrorCode() const
{
    return m_errorCode;
}

bdTaskResult* bdRemoteTask::getTaskResult()
{
    return m_taskResult;
}
