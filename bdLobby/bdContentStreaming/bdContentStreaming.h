// SPDX-License-Identifier: GPL-3.0-or-later

class bdContentStreaming : public bdContentStreamingBase
{
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdContentStreaming(bdRemoteTaskManager* const remoteTaskManager);
    ~bdContentStreaming();
    bdRemoteTaskRef upload(const bdUInt16 fileSlot, bdUploadInterceptor* uploadHandler, const bdUInt fileSize, const bdNChar8* fileName, const bdUInt16 category, void* thumbData,
        const bdUInt thumbDataSize, const bdUInt numTags, bdTag* tags, bdFileID* fileID);
    bdRemoteTaskRef upload(const bdUInt16 fileSlot, void* fileData , const bdUInt fileSize, const bdNChar8* fileName, const bdUInt16 category, void* thumbData,
        const bdUInt thumbDataSize, const bdUInt numTags, bdTag* tags, bdFileID* fileID);
    bdRemoteTaskRef copyFromPooledStorage(bdUInt64 pooledFileID, const bdUInt16 fileSlot, bdNChar8* fileName, const bdUInt16 category, void* metaData, bdUInt metaDataSize,
        bdUInt numTags, bdTag* tags, bdFileID* fileID);
    bdRemoteTaskRef copyFromUserStorage(bdUInt64 userFileID, const bdUInt16 fileSlot, bdNChar8* fileName, const bdUInt16 category, void* metaData, bdUInt metaDataSize,
        bdUInt numTags, bdTag* tags, bdFileID* fileID);
    bdRemoteTaskRef download(const bdUInt64 fileID, bdDownloadInterceptor* downloadHandler, bdFileMetaData* fileMetaData, bdUInt startByte, bdUInt endByte);
    bdRemoteTaskRef download(const bdUInt64 fileID, void* fileData, const bdUInt fileSize, bdFileMetaData* fileMetaData, bdUInt startByte, bdUInt endByte);
    bdRemoteTaskRef downloadFileBySlot(const bdUInt64, const bdUInt16, bdDownloadInterceptor*, bdFileMetaData*, bdUInt, bdUInt);
    bdRemoteTaskRef downloadFileBySlot(const bdUInt64, const bdUInt16, void*, const bdUInt, bdFileMetaData*, bdUInt, bdUInt);
    bdRemoteTaskRef downloadPublisherFile(const bdUInt64, bdDownloadInterceptor*, bdFileMetaData*, bdUInt, bdUInt);
    bdRemoteTaskRef downloadPublisherFile(const bdUInt64, void*, const bdUInt, bdFileMetaData*, bdUInt, bdUInt);
    bdRemoteTaskRef downloadApplePurchasedFile(const bdUInt64, void*, const bdUInt, bdDownloadInterceptor*, bdFileMetaData*, bdUInt, bdUInt);
    bdRemoteTaskRef downloadApplePurchasedFile(const bdUInt64, void*, const bdUInt, void*, const bdUInt, bdFileMetaData*, bdUInt, bdUInt);
    bdRemoteTaskRef removeFile(const bdUInt16 fileSlot);
    bdRemoteTaskRef getFileMetaDataByID(const bdUInt numFileIDs, bdUInt64* fileIDs, bdFileMetaData* fileDescriptor);
    bdRemoteTaskRef uploadUserSummaryMetaData(bdUInt64 fileID, void* summaryData, bdUInt summaryDataSize, void* metaData, bdUInt metaDataSize, bdUInt numTags, bdTag* tags);
    bdRemoteTaskRef downloadUserSummary(bdUInt64 fileID, void* summaryData, bdUInt summaryDataSize, bdUInt startByte, bdUInt endByte);
    bdRemoteTaskRef listAllPublisherFiles(const bdUInt, const bdUInt16, bdFileMetaData*, const bdUInt16, const bdUInt16, bdNChar8*);
    bdRemoteTaskRef listFilesByOwner(const bdUInt64 ownerID, const bdUInt startDate, const bdUInt16 category, bdFileMetaData* fileDescriptors,
        const bdUInt16 maxNumResults, const bdUInt16 offset, bdNChar8* fileName);
    bdRemoteTaskRef listFilesByOwners(bdUInt64*, bdUInt, const bdUInt, const bdUInt16, bdFileMetaData*, const bdUInt16, const bdUInt16, bdNChar8*);
    bdRemoteTaskRef reportContent(const bdUInt64);
    virtual bdUInt getMaxMetaDataSize();
protected:
    bdRemoteTaskRef _preUpload(const bdNChar8* filename, const bdUInt16 fileSlot, const bdUInt fileSize, const bdUInt16 category, bdURL* uploadURLs);
    bdRemoteTaskRef _preUploadSummary();
    virtual bdRemoteTaskRef _postUploadFile();
    virtual bdRemoteTaskRef _postUploadSummary();
    bdRemoteTaskRef _preCopy(const bdUInt64 fileID, const bdUByte8 taskID);
    virtual bdRemoteTaskRef _postCopy();
    bdRemoteTaskRef _preDownloadFileBySlot(bdUInt64 fileID, bdUInt16 fileSlot, bdUInt fileSize, bdFileMetaData* fileMetaData);
    bdRemoteTaskRef _preDownloadByFileID(bdUInt64 fileID, bdUInt fileSize, bdFileMetaData* fileMetaData);
    bdRemoteTaskRef _preDownloadPublisherFile(bdUInt64, bdUInt, bdFileMetaData*);
    bdRemoteTaskRef _preDownloadITunesPurchasedFile(bdUInt64, bdUInt, void*, const bdUInt, bdFileMetaData*);
    bdRemoteTaskRef _preDownloadSummary();
    bdRemoteTaskRef _preDeleteFile(bdUInt16 fileSlot, bdURL* deleteURLs);
};