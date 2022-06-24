// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

bdNChar8 const* const bdContentStreamingBase::statusStrings[6] = { "READY", "PRE_HTTP_OPERATION", "HTTP_OPERATION", "POST_HTTP_OPERATION", "DONE", "FAILED" };

void bdContentStreamingBase::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdContentStreamingBase::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdContentStreamingBase::bdContentStreamingBase(bdRemoteTaskManager* const remoteTaskManager)
    : m_operation(0), m_state(READY), m_taskData(), m_uploadData(NULL), m_uploadFileID(NULL), m_URLs(), m_thumbData(NULL), m_thumbDataSize(0), m_sendChecksum(false), m_downloadMetaData(NULL),
    m_downloadData(NULL), m_downloadDataSize(0), m_startByte(0), m_endByte(0), m_preCopyResults(), m_uploadSummary(false), m_summaryMeta(), m_overallTask(), m_remoteTask(), m_http(),
    m_md5Hash(), m_finalizeOnComplete(true), m_remoteTaskManager(remoteTaskManager), m_testURLs(NULL), m_testAddressCount(0)
{
    for (bdUInt i = 0; i < 3; ++i)
    {
        m_preCopyResults[i].m_source = &m_URLs[i];
    }
    bdMemset(m_checksum, 0, sizeof(m_checksum));
}

bdContentStreamingBase::~bdContentStreamingBase()
{
}

void bdContentStreamingBase::abortHTTPOperation()
{
    if (*m_overallTask)
    {
        if (m_state == HTTP_OPERATION)
        {
            if (!m_http.abortInProgress())
            {
                bdLogWarn("lobby/contentStreamingBase", "Aborting HTTP operation");
                m_http.abortOperation();
            }
        }
        else
        {
            bdLogWarn("lobby/contentStreamingBase", "Aborting HTTP operation while in state %s", statusStrings[m_state]);
        }
        m_overallTask->cancelTask();
        if (*m_remoteTask)
        {
            m_remoteTask->cancelTask();
        }
    }
}

void bdContentStreamingBase::checkProgress(bdUInt& bytesTransfered, bdFloat32& dataRate)
{
    bytesTransfered = m_http.getTransferProgress();
    dataRate = m_http.getTransferSpeed();
}

bdInt bdContentStreamingBase::getLastHTTPError()
{
    return m_http.getLastHTTPError();
}

void bdContentStreamingBase::pump()
{
    if (*m_overallTask && m_overallTask->getStatus() == bdRemoteTask::BD_CANCELLED)
    {
        abortHTTPOperation();
    }
    switch (m_state)
    {
    case bdContentStreamingBase::PRE_HTTP_OPERATION:
    {
        bdRemoteTask::bdStatus postState = m_remoteTask->getStatus();
        if (postState == bdRemoteTask::BD_DONE)
        {
            handlePreHTTPComplete();
        }
        else if (postState != bdRemoteTask::BD_PENDING)
        {
            setState(FAILED, BD_NO_ERROR);
        }
        break;
    }
    case bdContentStreamingBase::HTTP_OPERATION:
    {
        bdHTTPWrapperBase::bdStatus httpStatus = m_http.getStatus();
        if (httpStatus == bdHTTPWrapperBase::BD_DONE)
        {
            handleHTTPComplete();
        }
        else if (httpStatus > bdHTTPWrapperBase::BD_DONE && httpStatus <= bdHTTPWrapperBase::BD_FAILED_TO_START)
        {
            handleHTTPFailed();
        }
        break;
    }
    case bdContentStreamingBase::POST_HTTP_OPERATION:
    {
        bdRemoteTask::bdStatus preState = m_remoteTask->getStatus();
        if (preState != bdRemoteTask::BD_PENDING)
        {
            setState(FAILED, BD_NO_ERROR);
        }
        break;
    }
    default:
        break;
    }
    if (m_state == FAILED || m_state == DONE)
    {
        m_operation = 0;
    }
}

void bdContentStreamingBase::enableVerboseOutput(bdBool enable)
{
    m_http.enableVerboseOutput(enable);
}

void bdContentStreamingBase::enableProgressMeter(bdBool enable)
{
    m_http.enableProgressMeter(enable);
}

void bdContentStreamingBase::enablePersistentThread(bdBool enable)
{
}

void bdContentStreamingBase::setTestLSPServerAddresses(bdURL*, bdUInt)
{
}

void bdContentStreamingBase::handlePreHTTPComplete()
{
    if (m_useTestAddresses && m_downloadMetaData)
    {
        swapURLInfo(m_downloadMetaData->m_url);
    }
    if (m_useTestAddresses)
    {
        swapURLInfo(m_URLs[m_httpSite].m_url);
    }
    bdHTTPWrapperBase::bdStatus started = bdHTTPWrapperBase::BD_EMPTY;
    switch (m_operation)
    {
    case 'e':
        m_taskData.m_fileID = m_URLs[m_httpSite].m_fileID;
        if (m_uploadHandler)
        {
            bdLogInfo("lobby/contentStreamingBase", "HTTP PUT %s", m_URLs[m_httpSite].m_url);
            started = m_http._startUpload(m_URLs[m_httpSite].m_url, NULL, m_uploadHandler, m_taskData.m_fileSize, m_remoteTask->getTransactionID(), NULL);
            break;
        }
        bdLogInfo("lobby/contentStreamingBase", "HTTP PUT %s", m_URLs[m_httpSite].m_url);
        started = m_http._startUpload(m_URLs[m_httpSite].m_url, m_uploadData, NULL, m_taskData.m_fileSize, m_remoteTask->getTransactionID(), m_sendChecksum ? m_checksum : NULL);
        break;
    case 'h':
        bdLogInfo("lobby/contentStreamingBase", "HTTP COPY from %s", m_preCopyResults[m_httpSite].m_source->m_url);
        bdLogInfo("lobby/contentStreamingBase", "HTTP COPY to %s", m_preCopyResults[m_httpSite].m_destination);
        started = m_http.startCopy(m_preCopyResults[m_httpSite].m_source->m_url, m_preCopyResults[m_httpSite].m_destination, m_remoteTask->getTransactionID());
        break;
    case 'f':
        if (m_downloadHandler)
        {
            bdLogInfo("lobby/contentStreamingBase", "HTTP GET %s", m_downloadMetaData->m_url);
            started = m_http._startDownload(m_downloadMetaData->m_url, NULL, 0, m_downloadHandler, m_remoteTask->getTransactionID(), m_startByte, m_endByte);
            break;
        }
        bdLogInfo("lobby/contentStreamingBase", "HTTP GET %s", m_downloadMetaData->m_url);
        started = m_http._startDownload(m_downloadMetaData->m_url, m_downloadData, m_downloadDataSize, NULL, m_remoteTask->getTransactionID(), m_startByte, m_endByte);
        break;
    case 'g':
        bdLogInfo("lobby/contentStreamingBase", "HTTP DELETE %s", m_URLs[m_httpSite].m_url);
        started = m_http.startDelete(m_URLs[m_httpSite].m_url, m_remoteTask->getTransactionID());
        break;
    default:
        break;
    }
    if (started == bdHTTPWrapperBase::BD_PENDING || started == bdHTTPWrapperBase::BD_DONE)
    {
        setState(HTTP_OPERATION, BD_NO_ERROR);
    }
    else
    {
        setState(FAILED, BD_CONTENTSTREAMING_FAILED_TO_START_HTTP);
    }
}

void bdContentStreamingBase::handleHTTPComplete()
{
    switch (m_operation)
    {
    case 'e':
        bdLogInfo("lobby/contentStreamingBase", "Uploaded: \"%s\" @ %.2f KB/sec", m_taskData.m_fileName, m_http.getTransferSpeed() / 1024.0f);
        m_remoteTask = _postUpload();
        setState((*m_remoteTask) ? POST_HTTP_OPERATION : FAILED, BD_NO_ERROR);
        break;
    case 'h':
        if (++m_httpSite < m_remoteTask->getNumResults())
        {
            setState(PRE_HTTP_OPERATION, BD_NO_ERROR);
            break;
        }
        m_remoteTask = _postCopy();
        setState((*m_remoteTask) ? POST_HTTP_OPERATION : FAILED, BD_NO_ERROR);
        break;
    case 'f':
        bdLogInfo("lobby/contentStreamingBase", "Transfered: %d KB @ %0.2f KB/sec", m_http.getTransferProgress() >> 10, m_http.getTransferSpeed() / 1024.0f);
        setState(DONE, BD_NO_ERROR);
        m_overallTask->setNumResults(1);
        break;
    case 'g':
        if (++m_httpSite < m_remoteTask->getNumResults())
        {
            setState(PRE_HTTP_OPERATION, BD_NO_ERROR);
            break;
        }
        setState(DONE, BD_NO_ERROR);
        break;
    default:
        break;
    }
}

void bdContentStreamingBase::handleHTTPFailed()
{
    switch (m_operation)
    {
    case 'e':
        bdLogError("lobby/contentStreamingBase", "HTTP PUT failed for site %s", m_URLs[m_httpSite].m_url);
        if (++m_httpSite < m_remoteTask->getNumResults())
        {
            setState(PRE_HTTP_OPERATION, BD_NO_ERROR);
            break;
        }
        bdLogError("lobby/contentStreamingBase", "HTTP PUT failed");
        setState(FAILED, BD_CONTENTSTREAMING_HTTP_ERROR);
        break;
    case 'f':
        bdLogError("lobby/contentStreamingBase", "HTTP GET failed %s", m_downloadMetaData->m_url);
        setState(FAILED, BD_CONTENTSTREAMING_HTTP_ERROR);
        m_overallTask->setNumResults(0);
        break;
    case 'g':
        bdLogError("lobby/contentStreamingBase", "HTTP DELETE failed for site %s", m_URLs[m_httpSite].m_url);
        if (++m_httpSite < m_remoteTask->getNumResults())
        {
            setState(PRE_HTTP_OPERATION, BD_NO_ERROR);
            break;
        }
        bdLogError("lobby/contentStreamingBase", "HTTP DELETE failed");
        setState(FAILED, BD_CONTENTSTREAMING_HTTP_ERROR);
        break;
    case 'h':
        bdLogError("lobby/contentStreamingBase", "HTTP COPY failed for site %s", m_preCopyResults[m_httpSite].m_source->m_url);
        setState(FAILED, BD_CONTENTSTREAMING_HTTP_ERROR);
        m_overallTask->setNumResults(0);
        break;
    default:
        break;
    }
}

bdRemoteTaskRef bdContentStreamingBase::_postUpload()
{
    bdUInt actualUploaded = m_http.getTransferProgress();
    if (m_taskData.m_fileSize && m_taskData.m_fileSize != actualUploaded && m_http.getStatus() == bdHTTPWrapper::BD_DONE)
    {
        bdLogWarn("sim", "Upload completed with %d bytes sent. Expected %d", actualUploaded, m_taskData.m_fileSize);
    }
    m_taskData.m_fileSize = actualUploaded;
    bdRemoteTaskRef task((m_uploadSummary ? _postUploadSummary() : _postUploadFile()));
    m_uploadSummary = false;
    return task;
}

bdRemoteTaskRef bdContentStreamingBase::_postUploadFile()
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdContentStreamingBase::_postUploadSummary()
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdContentStreamingBase::_postCopy()
{
    return bdRemoteTaskRef();
}

bdBool bdContentStreamingBase::initUpload(const bdUInt16 fileSlot, const void* fileData, bdUploadInterceptor* uploadHandler, const bdUInt fileSize, const bdNChar8* const fileName, 
    const bdUInt16 category, const void* thumbData, const bdUInt thumbDataSize, const bdUInt numTags, const bdTag* tags, bdFileID* fileID, const bdUInt __formal, const bdBool isSummary)
{
    bdUByte8 md5[16];

    if (thumbDataSize > getMaxMetaDataSize())
    {
        bdLogError("lobby/contentStreamingBase", "Failed to init upload of meta data size %d (max %d)", thumbDataSize, getMaxMetaDataSize());
        return false;
    }
    if (!m_http.isOkayToStart())
    {
        bdLogError("lobby/contentStreamingBase", "BD_CONTENTSTREAMING_FAILED_TO_START_HTTP");
        return false;
    }

    m_uploadData = fileData;
    m_uploadHandler = uploadHandler;
    m_uploadFileID = fileID;
    m_uploadSummary = isSummary;
    m_thumbData = thumbData;
    m_thumbDataSize = thumbDataSize;
    m_taskData.m_fileSlot = fileSlot;
    m_taskData.m_fileSize = fileSize;
    m_taskData.m_category = category;
    m_taskData.m_numTags = numTags;
    m_taskData.m_fileID = fileID ? fileID->m_fileID : 0;
    
    if (fileName)
    {
        bdStrlcpy(m_taskData.m_fileName, fileName, bdStrlen(fileName) + 1);
    }
    else
    {
        m_taskData.m_fileName[0] = 0;
    }
    for (bdUInt i = 0; i < numTags; ++i)
    {
        m_taskData.m_tags[i] = tags[i];
    }
    m_sendChecksum = false;
    if (m_uploadData && m_taskData.m_fileSize)
    {
        bdUInt hashSize = 16;
        bdMemset(md5, 0, sizeof(md5));
        m_md5Hash.hash(reinterpret_cast<const bdUByte8*>(m_uploadData), m_taskData.m_fileSize, md5, &hashSize);
        m_sendChecksum = true;
        bdSnprintf(
            m_checksum,
            sizeof(m_checksum),
            "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
            md5[0],
            md5[1],
            md5[2],
            md5[3],
            md5[4],
            md5[5],
            md5[6],
            md5[7],
            md5[8],
            md5[9],
            md5[10],
            md5[11],
            md5[12],
            md5[13],
            md5[14],
            md5[15]);
    }
    m_httpSite = 0;
    return true;
}

bdBool bdContentStreamingBase::initDownload(void* fileData, bdUInt fileSize, bdDownloadInterceptor* downloadHandler, bdFileMetaData* fileMetaData, bdUInt startByte, bdUInt endByte)
{
    if (!m_http.isOkayToStart())
    {
        bdLogError("lobby/contentStreamingBase", "BD_CONTENTSTREAMING_FAILED_TO_START_HTTP");
        return false;
    }

    m_downloadData = fileData;
    m_downloadDataSize = fileSize;
    m_downloadHandler = downloadHandler;
    m_downloadMetaData = fileMetaData;
    m_startByte = startByte;
    m_endByte = endByte;
    return true;
}

bdBool bdContentStreamingBase::initDelete()
{
    if (m_http.isOkayToStart())
    {
        m_httpSite = 0;
        return true;
    }
    bdLogError("lobby/contentStreamingBase", "BD_CONTENTSTREAMING_FAILED_TO_START_HTTP");
    return false;
}

bdUInt bdContentStreamingBase::getMaxMetaDataSize()
{
    return 255;
}

bdRemoteTaskRef bdContentStreamingBase::startUpload()
{
    return start(101);
}

bdRemoteTaskRef bdContentStreamingBase::startCopy()
{
    return start(104);
}

bdRemoteTaskRef bdContentStreamingBase::startDownload()
{
    return start(102);
}

bdRemoteTaskRef bdContentStreamingBase::startDelete()
{
    return start(103);
}

bdRemoteTaskRef bdContentStreamingBase::start(bdUInt16 operation)
{
    m_operation = operation;
    m_http.resetTransferStats();
    m_overallTask = new bdRemoteTask();
    m_overallTask->setStatus(bdRemoteTask::BD_PENDING);
    setState(PRE_HTTP_OPERATION, BD_NO_ERROR);
    return bdRemoteTaskRef(m_overallTask);
}

void bdContentStreamingBase::setState(bdContentStreamingBase::bdStatus newState, bdLobbyErrorCode errorCode)
{
    if (newState == DONE)
    {
        m_overallTask->setStatus(bdRemoteTask::BD_DONE);
        m_overallTask->setErrorCode(BD_NO_ERROR);
    }
    else if (newState == FAILED)
    {
        m_overallTask->setStatus(bdRemoteTask::BD_FAILED);
        if (errorCode)
        {
            m_overallTask->setErrorCode(errorCode);
        }
        else if (m_state == PRE_HTTP_OPERATION || m_state == POST_HTTP_OPERATION)
        {
            m_overallTask->setErrorCode(m_remoteTask->getErrorCode());
        }
    }
    if (m_finalizeOnComplete
        && (newState == DONE || newState == FAILED)
        && m_state != DONE
        && m_state != FAILED)
    {
        m_http.finalize();
    }
    bdLogInfo("lobby/contentStreamingBase", "state %s -> %s", s_statusStrings[m_state], s_statusStrings[newState]);
    m_state = newState;
}

void bdContentStreamingBase::swapURLInfo(bdNChar8* url)
{
    // Not needed at the moment since we don't need to use the testing
}
