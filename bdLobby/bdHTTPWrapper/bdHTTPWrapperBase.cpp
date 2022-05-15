// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

bdHTTPWrapperBase::bdHTTPWrapperBase()
    : bdRunnable(), m_threadExiting(false), m_threadMutex(), m_thread(NULL), m_status(BD_EMPTY), m_operation(BD_IDLE), m_httpSema(new bdSemaphore(0, 1u))
{
}

bdHTTPWrapperBase::~bdHTTPWrapperBase()
{
    m_httpSema->destroy();
}

bdHTTPWrapperBase::bdStatus bdHTTPWrapperBase::getStatus()
{
    return m_status;
}

bdBool bdHTTPWrapperBase::isOkayToStart()
{
    return m_operation == BD_IDLE;
}

void bdHTTPWrapperBase::finalize()
{
    m_threadMutex.lock();
    if (m_thread && !m_threadExiting)
    {
        m_threadExiting = true;
        m_threadMutex.unlock();
        m_httpSema->release();
        m_thread->join();
        m_threadMutex.unlock();
        m_thread->cleanup();
        m_thread = NULL;
        m_threadExiting = false;
    }
    m_threadMutex.unlock();
}

bdBool bdHTTPWrapperBase::startAsyncOperation(bdHTTPWrapperBase::bdOperation op)
{
    bdBool success = true;
    m_threadMutex.lock();
    if (m_threadExiting)
    {
        success = false;
    }
    else if (!m_thread)
    {
        m_thread = new bdThread(this, 0, 0x8000);
        success = m_thread->start(NULL, NULL);
    }
    m_threadMutex.unlock();
    if (success)
    {
        m_status = BD_PENDING;
        m_operation = op;
        m_httpSema->release();
    }
    else
    {
        bdLogError("http", "Unable to start thread for HTTP operations");
        m_status = BD_FAILED;
        m_operation = BD_IDLE;
    }
    return success;
}

bdBool bdHTTPWrapperBase::parseURL(const bdNChar8* const url, bdNChar8* serverName, bdNChar8** startRequestPath, bdUInt32* port)
{
    const bdNChar8* endServerName;
    const bdNChar8* startServerName;

    if (bdStrncmp(url, "http", 4))
    {
        bdLogError("http", "URL was not of expected form. expected: http(s)://serverName:port/resource\nactual: %s", url);
        return false;
    }
    startServerName = bdStrchr(url, ':');
    if (!startServerName)
    {
        bdLogError("http", "URL was not of expected form. expected: http(s)://serverName:port/resource\nactual: %s", url);
        return false;
    }
    startServerName += bdStrlen("://");
    *startRequestPath = const_cast<bdNChar8*>(bdStrchr(startServerName, '/'));
    if (!*startRequestPath)
    {
        bdLogError("http", "URL was not of expected form. expected: http(s)://serverName:port/resource\nactual: %s", url);
        return false;
    }
    endServerName = bdStrchr(startServerName, ':');
    if (endServerName)
    {
        *port = atoi(endServerName-- + 1);
        if (startServerName > endServerName || endServerName > *startRequestPath)
        {
            bdLogError("http", "URL was not of expected form. expected: http(s)://serverName:port/resource\nactual: %s", url);
            return false;
        }
        if (!*port)
        {
            bdLogError("http", "URL was not of expected form. expected: http(s)://serverName:port/resource\nactual: %s", url);
            return false;
        }
    }
    else
    {
        *port = 80;
        endServerName = *startRequestPath - 1;
    }
    bdUInt len = endServerName - startServerName + 2;
    if (len < 0 && len > BD_MAX_SERVER_HOST_SIZE)
    {
        bdLogError("http", "URL host greater than BD_MAX_SERVER_HOST_SIZE. ('%s')", url);
        return false;
    }
    bdStrlcpy(serverName, startServerName, len);
    return true;
}
