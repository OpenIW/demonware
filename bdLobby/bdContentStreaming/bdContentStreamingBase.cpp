// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

bdNChar8 const* const bdContentStreamingBase::statusStrings[] = {"READY", "PRE_HTTP_OPERATION", "HTTP_OPERATION", "POST_HTTP_OPERATION", "DONE", "FAILED" };

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

void bdContentStreamingBase::checkProgress(bdUInt* bytesTransfered, bdFloat32* dataRate)
{
    *bytesTransfered = m_http.getTransferProgress();
    *dataRate = m_http.getTransferSpeed();
}

int bdContentStreamingBase::getLastHTTPError()
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
}

void bdContentStreamingBase::handleHTTPFailed()
{
}

bdRemoteTaskRef bdContentStreamingBase::_postUpload()
{
    return bdRemoteTaskRef();
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

bool bdContentStreamingBase::initUpload(const bdUInt16 fileSlot, const void* fileData, bdUploadInterceptor* uploadHandler, const bdUInt fileSize, const bdNChar8* const fileName, const bdUInt16 category, const void* thumbData, const bdUInt thumbDataSize, const bdUInt numTags, const bdTag* tags, bdFileID* fileID, const bdUInt __formal, const bool isSummary)
{
    return false;
}

bool bdContentStreamingBase::initDownload(void* fileData, bdUInt fileSize, bdDownloadInterceptor* downloadHandler, bdFileMetaData* fileMetaData, bdUInt startByte, bdUInt endByte)
{
    return false;
}

bool bdContentStreamingBase::initDelete()
{
    return false;
}

unsigned int bdContentStreamingBase::getMaxMetaDataSize()
{
    return 0;
}

bdRemoteTaskRef bdContentStreamingBase::startUpload()
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdContentStreamingBase::startCopy()
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdContentStreamingBase::startDownload()
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdContentStreamingBase::startDelete()
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdContentStreamingBase::start(bdUInt16 operation)
{
    return bdRemoteTaskRef();
}

void bdContentStreamingBase::setState(bdContentStreamingBase::bdStatus newState, bdLobbyErrorCode errorCode)
{
}

void bdContentStreamingBase::swapURLInfo(bdNChar8* url)
{
}
