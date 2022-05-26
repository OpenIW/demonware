// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdHTTPWrapper::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdHTTPWrapper::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdHTTPWrapper::bdHTTPWrapper()
    : bdHTTPWrapperBase(), m_httpClient()
{
}

bdHTTPWrapper::~bdHTTPWrapper()
{
    abortOperation();
    finalize();
    m_httpClient.~bdHTTPClient();
}

bdHTTPWrapperBase::bdStatus bdHTTPWrapper::startDownload(const bdNChar8* const downloadURL, bdDownloadInterceptor* downloadHandler, bdUInt64 transactionID, bdUInt startByte, bdUInt endByte)
{
    return bdHTTPWrapperBase::bdStatus();
}

bdHTTPWrapperBase::bdStatus bdHTTPWrapper::startDownload(const bdNChar8* const downloadURL, void* const downloadBuffer, bdUInt bufferSize, bdUInt64 transactionID, bdUInt startByte, bdUInt endByte)
{
    return bdHTTPWrapperBase::bdStatus();
}

bdHTTPWrapperBase::bdStatus bdHTTPWrapper::startUpload(const bdNChar8* const uploadURL, bdUploadInterceptor* uploadHandler, bdUInt uploadSize, bdUInt64 transactionID)
{
    return bdHTTPWrapperBase::bdStatus();
}

bdHTTPWrapperBase::bdStatus bdHTTPWrapper::startUpload(const bdNChar8* const uploadURL, const void* const uploadBuffer, bdUInt uploadSize, bdUInt64 transactionID, bdNChar8* checkSum)
{
    return bdHTTPWrapperBase::bdStatus();
}

bdHTTPWrapperBase::bdStatus bdHTTPWrapper::startCopy(const bdNChar8* const copyURL, const bdNChar8* destination, bdUInt64 transactionID)
{
    m_status = BD_FAILED;

    bdNChar8 serverName[3088];
    bdNChar8* requestPath;
    bdUInt port;

    m_status = BD_FAILED;
    requestPath = NULL;
    if (parseURL(copyURL, serverName, &requestPath, port)
        && m_httpClient.httpCopy(serverName, requestPath, destination, port, transactionID))
    {
        startAsyncOperation(BD_COPY);
    }
    return m_status;
}

bdHTTPWrapperBase::bdStatus bdHTTPWrapper::startDelete(const bdNChar8* const deleteURL, bdUInt64 transactionID)
{
    char serverName[388];
    char* requestPath;
    unsigned int port;

    this->m_status = BD_FAILED;
    requestPath = 0;
    if (parseURL(deleteURL, serverName, &requestPath, port)
        && m_httpClient.httpDelete(serverName, requestPath, port, transactionID))
    {
        startAsyncOperation(BD_DELETE);
    }
    return this->m_status;
}

void bdHTTPWrapper::enableVerboseOutput(bdBool enable)
{
    m_httpClient.enableVerboseOutput(enable);
}

void bdHTTPWrapper::enableProgressMeter(bdBool enable)
{
    m_httpClient.enableProgressMeter(enable);
}

void bdHTTPWrapper::abortOperation()
{
    m_httpClient.abortOperation();
}

bdBool bdHTTPWrapper::abortInProgress()
{
    return m_httpClient.isAborted();
}

bdUInt bdHTTPWrapper::getTransferProgress()
{
    return m_httpClient.getBytesTransfered();
}

bdFloat32 bdHTTPWrapper::getTransferSpeed()
{
    return m_httpClient.getDataRate();
}

void bdHTTPWrapper::resetTransferStats()
{
    m_httpClient.setBytesTransfered(0);
    m_httpClient.setDataRate(0.0f);
}

bdInt bdHTTPWrapper::getLastHTTPError()
{
    if (m_status == BD_PENDING)
    {
        return 0;
    }
    return m_httpClient.getResponseCode();
}

bdUInt bdHTTPWrapper::run(void* args)
{
    do
    {
        m_httpSema->wait();
        if (m_httpClient.getStatus() == bdHTTPClient::BD_HTTP_STATUS_BUSY)
        {
            m_httpClient.performOperation();
            if (m_httpClient.isAborted())
            {
                m_status = BD_FAILED;
            }
            else if (m_httpClient.getStatus() == bdHTTPClient::BD_HTTP_STATUS_DONE)
            {
                m_status = BD_DONE;
            }
            else
            {
                m_status = BD_FAILED;
            }
            m_operation = BD_IDLE;
        }
    } while (!m_threadExiting);
    return 0;
}

bdHTTPWrapperBase::bdStatus bdHTTPWrapper::_startDownload(const bdNChar8* const downloadURL, void* const downloadBuffer, bdUInt bufferSize, bdDownloadInterceptor* downloadHandler, 
    bdUInt64 transactionID, bdUInt startByte, bdUInt endByte)
{
    bdNChar8 serverName[3088];
    bdNChar8* requestPath;
    bdUInt port;

    m_status = BD_FAILED;
    requestPath = NULL;
    if (parseURL(downloadURL, serverName, &requestPath, port)
        && m_httpClient.httpGet(serverName, requestPath, downloadBuffer, bufferSize, downloadHandler, port, transactionID, startByte, endByte))
    {
        startAsyncOperation(BD_DOWNLOAD);
    }
    return m_status;
}

bdHTTPWrapperBase::bdStatus bdHTTPWrapper::_startUpload(const bdNChar8* const uploadURL, const void* const fileData, bdUploadInterceptor* uploadHandler, bdUInt uploadSize, 
    bdUInt64 transactionID, bdNChar8* checkSum)
{
    bdNChar8 serverName[388];
    bdNChar8* requestPath;
    bdUInt port;

    m_status = BD_FAILED;
    requestPath = NULL;
    if (parseURL(uploadURL, serverName, &requestPath, port)
        && m_httpClient.httpPut(serverName, requestPath, fileData, uploadSize, uploadHandler, port, transactionID, checkSum))
    {
        startAsyncOperation(BD_UPLOAD);
    }
    return m_status;
}
