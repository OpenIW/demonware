// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdContentStreamingBase
{
public:
    enum bdStatus
    {
        READY = 0,
        PRE_HTTP_OPERATION = 1,
        HTTP_OPERATION = 2,
        POST_HTTP_OPERATION = 3,
        DONE = 4,
        FAILED = 5
    };
    static bdNChar8 const* const statusStrings[];
protected:
    bdUInt m_operation;
    bdContentStreamingBase::bdStatus m_state;
    bdUploadInterceptor* m_uploadHandler;
    bdFileMetaData m_taskData;
    const void* m_uploadData;
    bdFileID* m_uploadFileID;
    bdURL m_URLs[3];
    const void* m_thumbData;
    bdUInt m_thumbDataSize;
    bdUInt m_httpSite;
    bdBool m_sendChecksum;
    bdNChar8 m_checksum[33];
    bdDownloadInterceptor* m_downloadHandler;
    bdFileMetaData* m_downloadMetaData;
    void* m_downloadData;
    bdUInt m_downloadDataSize;
    bdUInt m_startByte;
    bdUInt m_endByte;
    bdPreCopyResult m_preCopyResults[3];
    bdBool m_uploadSummary;
    bdSummaryMetaHandler m_summaryMeta;
    bdRemoteTaskRef m_overallTask;
    bdRemoteTaskRef m_remoteTask;
    bdHTTPWrapper m_http;
    bdHashMD5 m_md5Hash;
    bdURL* m_testURLs;
    bdUInt m_testAddressCount;
    bdBool m_useTestAddresses;
    bdBool m_finalizeOnComplete;
    bdRemoteTaskManager* m_remoteTaskManager;
    bdNChar8* s_statusStrings[6];
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdContentStreamingBase(bdRemoteTaskManager* const remoteTaskManager);
    virtual ~bdContentStreamingBase();
    void abortHTTPOperation();
    void checkProgress(bdUInt* bytesTransfered, bdFloat32* dataRate);
    bdInt getLastHTTPError();
    void pump();
    void enableVerboseOutput(bdBool enable);
    void enableProgressMeter(bdBool enable);
    void enablePersistentThread(bdBool enable);
    void setTestLSPServerAddresses(bdURL*, bdUInt);
protected:
    void handlePreHTTPComplete();
    void handleHTTPComplete();
    void handleHTTPFailed();
    bdRemoteTaskRef _postUpload();
    virtual bdRemoteTaskRef _postUploadFile();
    virtual bdRemoteTaskRef _postUploadSummary();
    virtual bdRemoteTaskRef _postCopy();
    bdBool initUpload(const bdUInt16 fileSlot, const void* fileData, bdUploadInterceptor* uploadHandler, const bdUInt fileSize, const bdNChar8* const fileName,
        const bdUInt16 category, const void* thumbData, const bdUInt thumbDataSize, const bdUInt numTags, const bdTag* tags, bdFileID* fileID, const bdUInt __formal, const bool isSummary);
    bdBool initDownload(void* fileData, bdUInt fileSize, bdDownloadInterceptor* downloadHandler, bdFileMetaData* fileMetaData, bdUInt startByte, bdUInt endByte);
    bdBool initDelete();
    virtual bdUInt getMaxMetaDataSize();
    bdRemoteTaskRef startUpload();
    bdRemoteTaskRef startCopy();
    bdRemoteTaskRef startDownload();
    bdRemoteTaskRef startDelete();
    bdRemoteTaskRef start(bdUInt16 operation);
    void setState(bdContentStreamingBase::bdStatus newState, bdLobbyErrorCode errorCode);
    void swapURLInfo(bdNChar8* url);
};