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
protected:
    bdUInt m_operation;
    bdContentStreamingBase::bdStatus m_state;
    bdUploadInterceptor* m_uploadHandler;
    bdFileMetaData m_taskData;
    void* m_uploadData;
    bdFileID* m_uploadFileID;
    bdURL m_URLs[3];
    void* m_thumbData;
    bdUInt m_thumbDataSize;
    bdUInt m_httpSite;
    bdBool m_sendChecksum;
    bdUInt m_checksum[33];
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
    bdNChar8* s_statusStrings[0];
public:
    bdContentStreamingBase(bdContentStreamingBase* other);
    bdContentStreamingBase(const bdRemoteTaskManager*);
    ~bdContentStreamingBase();
    void abortHTTPOperation();
    void checkProgress(bdUInt*, bdFloat32*);
    int getLastHTTPError();
    void pump();
    void enableVerboseOutput(bdBool);
    void enableProgressMeter(bdBool);
    void enablePersistentThread(bdBool);
    void setTestLSPServerAddresses(bdURL*, bdUInt);
protected:
    void handlePreHTTPComplete();
    void handleHTTPComplete();
    void handleHTTPFailed();
    bdRemoteTaskRef _postUpload();
    bdRemoteTaskRef _postUploadFile();
    bdRemoteTaskRef _postUploadSummary();
    bdRemoteTaskRef _postCopy();
    bool initUpload(const unsigned int, void*, bdUploadInterceptor*, const unsigned int, const char*, const unsigned int, void*, const unsigned int,
        const unsigned int, bdTag*, bdFileID*, const unsigned int, const bool);
    bool initDownload(void*, unsigned int, bdDownloadInterceptor*, bdFileMetaData*, unsigned int, unsigned int);
    bool initDelete();
    unsigned int getMaxMetaDataSize();
    bdRemoteTaskRef startUpload();
    bdRemoteTaskRef startCopy();
    bdRemoteTaskRef startDownload();
    bdRemoteTaskRef startDelete();
    bdRemoteTaskRef start(unsigned int);
    void setState(bdContentStreamingBase::bdStatus, bdLobbyErrorCode);
    void swapURLInfo(char*);
};