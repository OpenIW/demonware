// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#define BD_MAX_SERVER_HOST_SIZE 384

class bdHTTPWrapperBase : public bdRunnable
{
public:
    enum bdStatus
    {
        BD_EMPTY = 0,
        BD_PENDING = 1,
        BD_DONE = 2,
        BD_FAILED = 3,
        BD_TIMED_OUT = 4,
        BD_FAILED_TO_START = 5,
        BD_MAX_STATUS = 6
    };
    enum bdOperation
    {
        BD_IDLE = 0,
        BD_UPLOAD = 1,
        BD_DOWNLOAD = 2,
        BD_DELETE = 3,
        BD_COPY = 4
    };
protected:
    bdBool m_threadExiting;
    bdMutex m_threadMutex;
    bdSemaphore* m_httpSema;
    bdThread* m_thread;
    bdHTTPWrapperBase::bdStatus m_status;
    bdHTTPWrapperBase::bdOperation m_operation;
public:
    bdHTTPWrapperBase();
    ~bdHTTPWrapperBase();
    bdHTTPWrapperBase::bdStatus getStatus();
    bdBool isOkayToStart();
    void finalize();
protected:
    bdBool startAsyncOperation(bdHTTPWrapperBase::bdOperation op);
    bdBool parseURL(const bdNChar8* const url, bdNChar8* serverName, bdNChar8** startRequestPath, bdUInt32& port);
};
