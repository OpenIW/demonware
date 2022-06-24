// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdPooledStorage : public bdContentStreamingBase
{
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdPooledStorage(bdRemoteTaskManager* const remoteTaskManager);
    ~bdPooledStorage();
    bdRemoteTaskRef remove(bdUInt64 fileID);
    bdRemoteTaskRef upload(bdUploadInterceptor* uploadHandle, bdUInt16 category, const bdNChar8* fileName, bdUInt numTags, bdTag* tags, bdFileID* fileID);
    bdRemoteTaskRef download(bdUInt64 fileID, bdDownloadInterceptor* downloadHandler, bdFileMetaData* pooledMetaData, bdUInt startByte, bdUInt endByte);
    bdRemoteTaskRef uploadSummaryMetaData(bdUInt64 fileID, const void* summaryData, bdUInt summaryDataSize, const void* metaData, bdUInt metaDataSize, bdUInt numTags, bdTag* tags);
    bdRemoteTaskRef downloadSummary(bdUInt64 fileID, void* summaryData, bdUInt summaryDataSize, bdUInt startByte, bdUInt endByte);
    bdRemoteTaskRef getPooledMetaDataByID(const bdUInt numFiles, const bdUInt64* fileIDs, bdFileMetaData* fileMetas);
protected:
    virtual bdRemoteTaskRef _postUploadFile();
    virtual bdRemoteTaskRef _postUploadSummary();
    bdRemoteTaskRef _preUpload();
    bdRemoteTaskRef _preDownload();
    bdRemoteTaskRef _preUploadSummary();
    bdRemoteTaskRef _preDownloadSummary();
    virtual bdUInt getMaxMetaDataSize();
};