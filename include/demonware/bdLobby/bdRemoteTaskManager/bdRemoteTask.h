// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdRemoteTask : public bdReferencable
{
public:
    enum bdStatus : bdInt
    {
        BD_EMPTY = 0x0,
        BD_PENDING = 0x1,
        BD_DONE = 0x2,
        BD_FAILED = 0x3,
        BD_TIMED_OUT = 0x4,
        BD_CANCELLED = 0x5,
        BD_MAX_STATUS = 0x6,
    };
protected:
    bdStopwatch m_timer;
    bdFloat32 m_timeout;
    bdRemoteTask::bdStatus m_status;
    bdByteBufferRef m_byteResults;
    bdTaskResult* m_taskResult;
    bdTaskResult** m_taskResultList;
    bdUInt m_numResults;
    bdUInt m_maxNumResults;
    bdUInt m_totalNumResults;
    bdUInt64 m_transactionID;
    bdLobbyErrorCode m_errorCode;
    bdTaskResultProcessor* m_taskResultProcessor;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdRemoteTask();
    bdRemoteTask(bdRemoteTask::bdStatus status);
    ~bdRemoteTask();

    void cancelTask();
    void handleTaskReply(bdByteBufferRef buffer);
    void handleAsyncTaskReply(bdByteBufferRef buffer);
    bdBool deserializeTaskReply(bdByteBufferRef buffer);
    void start(const bdFloat32 timeout);
    void stop(bdByteBufferRef results, const bdUInt64 id);

    void setNumResults(bdUInt numResults);
    void setStatus(const bdRemoteTask::bdStatus status);
    void setErrorCode(const bdLobbyErrorCode errorCode);
    void setTimeout(const bdFloat32 timeout);
    void setTaskResult(bdTaskResult* result, const bdUInt numResults);
    void setTaskResultList(bdTaskResult** result, const bdUInt numResults);
    void setTaskResultProcessor(bdTaskResultProcessor* processor);

    bdRemoteTask::bdStatus getStatus();
    bdUInt getNumResults() const;
    bdUInt getTotalNumResults() const;
    bdByteBufferRef getByteResults() const;
    bdStopwatch getTimer() const;
    bdFloat32 getTimeout() const;
    bdUInt64 getTransactionID() const;
    bdLobbyErrorCode getErrorCode() const;
    bdTaskResult* getTaskResult();
};

typedef bdReference<bdRemoteTask> bdRemoteTaskRef;
