// SPDXyLicense-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdContentStreaming::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdContentStreaming::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdContentStreaming::bdContentStreaming(bdRemoteTaskManager* const remoteTaskManager)
    : bdContentStreamingBase(remoteTaskManager)
{
}

bdContentStreaming::~bdContentStreaming()
{
}

bdRemoteTaskRef bdContentStreaming::upload(const bdUInt16 fileSlot, bdUploadInterceptor* uploadHandler, const bdUInt fileSize, const bdNChar8* fileName, const bdUInt16 category,
    void* thumbData, const bdUInt thumbDataSize, const bdUInt numTags, bdTag* tags, bdFileID* fileID)
{
    if (!initUpload(fileSlot, NULL, uploadHandler, fileSize, fileName, category, thumbData, thumbDataSize, numTags, tags, fileID, 0, false))
    {
        return bdRemoteTaskRef();
    }
    m_remoteTask = _preUpload(fileName, fileSlot, fileSize, category, m_URLs);
    if (m_remoteTask->getStatus() == bdRemoteTask::BD_PENDING)
    {
        return startUpload();
    }
    else
    {
        return bdRemoteTaskRef(m_remoteTask);
    }
}

bdRemoteTaskRef bdContentStreaming::upload(const bdUInt16 fileSlot, void* fileData, const bdUInt fileSize, const bdNChar8* fileName, const bdUInt16 category,
    void* thumbData, const bdUInt thumbDataSize, const bdUInt numTags, bdTag* tags, bdFileID* fileID)
{
    if (!initUpload(fileSlot, fileData, NULL, fileSize, fileName, category, thumbData, thumbDataSize, numTags, tags, fileID, 0, false))
    {
        return bdRemoteTaskRef();
    }
    m_remoteTask = _preUpload(fileName, fileSlot, fileSize, category, m_URLs);
    if (m_remoteTask->getStatus() == bdRemoteTask::BD_PENDING)
    {
        return startUpload();
    }
    else
    {
        return bdRemoteTaskRef(m_remoteTask);
    }
}

bdRemoteTaskRef bdContentStreaming::copyFromPooledStorage(bdUInt64 pooledFileID, const bdUInt16 fileSlot, bdNChar8* fileName, const bdUInt16 category, void* metaData,
    bdUInt metaDataSize, bdUInt numTags, bdTag* tags, bdFileID* fileID)
{
    if (!initUpload(fileSlot, NULL, NULL, 0, fileName, category, metaData, metaDataSize, numTags, tags, fileID, 0, false))
    {
        return bdRemoteTaskRef();
    }
    m_remoteTask = _preCopy(pooledFileID, 15);
    if (m_remoteTask->getStatus() == bdRemoteTask::BD_PENDING)
    {
        return startCopy();
    }
    else
    {
        return bdRemoteTaskRef(m_remoteTask);
    }
}

bdRemoteTaskRef bdContentStreaming::copyFromUserStorage(bdUInt64 userFileID, const bdUInt16 fileSlot, bdNChar8* fileName, const bdUInt16 category, void* metaData,
    bdUInt metaDataSize, bdUInt numTags, bdTag* tags, bdFileID* fileID)
{
    if (!initUpload(fileSlot, NULL, NULL, 0, fileName, category, metaData, metaDataSize, numTags, tags, fileID, 0, false))
    {
        return bdRemoteTaskRef();
    }
    m_remoteTask = _preCopy(userFileID, 20);
    if (m_remoteTask->getStatus() == bdRemoteTask::BD_PENDING)
    {
        return startCopy();
    }
    else
    {
        return bdRemoteTaskRef(m_remoteTask);
    }
}

bdRemoteTaskRef bdContentStreaming::download(const bdUInt64 fileID, bdDownloadInterceptor* downloadHandler, bdFileMetaData* fileMetaData, bdUInt startByte, bdUInt endByte)
{
    if (!initDownload(NULL, 0, downloadHandler, fileMetaData, startByte, endByte))
    {
        return bdRemoteTaskRef();
    }
    m_remoteTask = _preDownloadByFileID(fileID, 0x40000000, m_downloadMetaData);
    if (m_remoteTask->getStatus() == bdRemoteTask::BD_PENDING)
    {
        return startDownload();
    }
    else
    {
        return bdRemoteTaskRef(m_remoteTask);
    }
}

bdRemoteTaskRef bdContentStreaming::download(const bdUInt64 fileID, void* fileData, const bdUInt fileSize, bdFileMetaData* fileMetaData, bdUInt startByte, bdUInt endByte)
{
    if (!initDownload(fileData, fileSize, NULL, fileMetaData, startByte, endByte))
    {
        return bdRemoteTaskRef();
    }
    m_remoteTask = _preDownloadByFileID(fileID, fileSize, m_downloadMetaData);
    if (m_remoteTask->getStatus() == bdRemoteTask::BD_PENDING)
    {
        return startDownload();
    }
    else
    {
        return bdRemoteTaskRef(m_remoteTask);
    }
}

bdRemoteTaskRef bdContentStreaming::downloadFileBySlot(const bdUInt64, const bdUInt16, bdDownloadInterceptor*, bdFileMetaData*, bdUInt, bdUInt)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdContentStreaming::downloadFileBySlot(const bdUInt64, const bdUInt16, void*, const bdUInt, bdFileMetaData*, bdUInt, bdUInt)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdContentStreaming::downloadPublisherFile(const bdUInt64, bdDownloadInterceptor*, bdFileMetaData*, bdUInt, bdUInt)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdContentStreaming::downloadPublisherFile(const bdUInt64, void*, const bdUInt, bdFileMetaData*, bdUInt, bdUInt)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdContentStreaming::downloadApplePurchasedFile(const bdUInt64, void*, const bdUInt, bdDownloadInterceptor*, bdFileMetaData*, bdUInt, bdUInt)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdContentStreaming::downloadApplePurchasedFile(const bdUInt64, void*, const bdUInt, void*, const bdUInt, bdFileMetaData*, bdUInt, bdUInt)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdContentStreaming::removeFile(const bdUInt16 fileSlot)
{
    if (!initDelete())
    {
        return bdRemoteTaskRef();
    }
    m_remoteTask = _preDeleteFile(fileSlot, m_URLs);
    if (m_remoteTask->getStatus() == bdRemoteTask::BD_PENDING)
    {
        return startDelete();
    }
    else
    {
        return bdRemoteTaskRef(m_remoteTask);
    }
}

bdRemoteTaskRef bdContentStreaming::getFileMetaDataByID(const bdUInt numFileIDs, bdUInt64* fileIDs, bdFileMetaData* fileDescriptor)
{
    bdRemoteTaskRef task;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(9 * numFileIDs + 69, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 50, 1);
    buffer->writeUInt32(numFileIDs);
    for (bdUInt i = 0; i < numFileIDs; ++i)
    {
        buffer->writeUInt64(fileIDs[i]);
    }
    m_remoteTaskManager->startTask(task, buffer);
    task->setTaskResult(fileDescriptor, numFileIDs);
    return task;
}

bdRemoteTaskRef bdContentStreaming::uploadUserSummaryMetaData(bdUInt64 fileID, void* summaryData, bdUInt summaryDataSize, void* metaData, bdUInt metaDataSize, bdUInt numTags, bdTag* tags)
{
    if (!initUpload(0, summaryData, NULL, summaryDataSize, NULL, 0, metaData, metaDataSize, numTags, tags, NULL, 0, false))
    {
        return bdRemoteTaskRef();
    }
    m_taskData.m_fileID = fileID;
    m_uploadSummary = true;
    m_remoteTask = _preUploadSummary();
    if (m_remoteTask->getStatus() == bdRemoteTask::BD_PENDING)
    {
        return startUpload();
    }
    else
    {
        return bdRemoteTaskRef(m_remoteTask);
    }
}

bdRemoteTaskRef bdContentStreaming::downloadUserSummary(bdUInt64 fileID, void* summaryData, bdUInt summaryDataSize, bdUInt startByte, bdUInt endByte)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdContentStreaming::listAllPublisherFiles(const bdUInt, const bdUInt16, bdFileMetaData*, const bdUInt16, const bdUInt16, bdNChar8*)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdContentStreaming::listFilesByOwner(const bdUInt64 ownerID, const bdUInt startDate, const bdUInt16 category, bdFileMetaData* fileDescriptors,
    const bdUInt16 maxNumResults, const bdUInt16 offset, bdNChar8* fileName)
{
    bdRemoteTaskRef task;
    bdUInt taskSize = 87;
    if (fileName)
    {
        taskSize += bdStrnlen(fileName, 128) + 2;
    }
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 50, 2);
    buffer->writeUInt64(ownerID);
    buffer->writeUInt32(startDate);
    buffer->writeUInt16(category);
    buffer->writeUInt16(maxNumResults);
    buffer->writeUInt16(offset);
    if (fileName)
    {
        buffer->writeString(fileName, 128 + 1);
    }
    m_remoteTaskManager->startTask(task, buffer);
    task->setTaskResult(fileDescriptors, maxNumResults);
    return task;
}

bdRemoteTaskRef bdContentStreaming::listFilesByOwners(bdUInt64* ownerIDs, bdUInt numOwners, const bdUInt startDate, const bdUInt16 category, bdFileMetaData* fileDescriptors,
    const bdUInt16 maxNumResults, const bdUInt16 offset, bdNChar8* fileName)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdContentStreaming::reportContent(const bdUInt64)
{
    return bdRemoteTaskRef();
}

bdUInt bdContentStreaming::getMaxMetaDataSize()
{
    return 255;
}

bdRemoteTaskRef bdContentStreaming::_preUpload(const bdNChar8* filename, const bdUInt16 fileSlot, const bdUInt fileSize, const bdUInt16 category, bdURL* uploadURLs)
{
    bdRemoteTaskRef task;
    bdUInt taskSize = (filename ? bdStrnlen(filename, 128) + 2 : 0) + 112;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));

    m_remoteTaskManager->initTaskBuffer(buffer, 50, 5);
    buffer->writeString(filename, 128 + 1);
    buffer->writeUInt16(fileSlot);
    buffer->writeUInt32(fileSize);
    buffer->writeUInt16(category);
    if (m_checksum)
    {
        buffer->writeBlob(m_checksum, 32);
    }
    m_remoteTaskManager->startTask(task, buffer);
    task->setTaskResult(uploadURLs, 3);
    return task;
}

bdRemoteTaskRef bdContentStreaming::_preUploadSummary()
{
    bdRemoteTaskRef task;
    bdUInt taskSize = m_thumbDataSize + (16 * m_taskData.m_numTags) + 94;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));

    m_remoteTaskManager->initTaskBuffer(buffer, 50, 17);
    buffer->writeUInt64(m_taskData.m_fileID);
    buffer->writeInt32(m_taskData.m_fileSize);
    buffer->writeBlob(m_thumbData, m_thumbDataSize);
    buffer->writeArrayStart(10, 2 * m_taskData.m_numTags, sizeof(bdUInt64));
    for (bdUInt i = 0; i < m_taskData.m_numTags; ++i)
    {
        buffer->writeUInt64(m_taskData.m_tags[i].m_priTag);
        buffer->writeUInt64(m_taskData.m_tags[i].m_secTag);
    }
    buffer->writeArrayEnd();
    if (m_sendChecksum)
    {
        buffer->writeBlob(m_checksum, 32u);
    }
    m_remoteTaskManager->startTask(task, buffer);
    task->setTaskResult(m_URLs, 3);
    return task;
}

bdRemoteTaskRef bdContentStreaming::_postUploadFile()
{
    bdRemoteTaskRef task;
    bdUInt taskSize = 64;

    taskSize += (m_taskData.m_fileName[0] ? bdStrnlen(m_taskData.m_fileName, 128) + 2 : 0) + 6;
    taskSize += (m_URLs[m_httpSite].m_serverIndex[0] ? bdStrnlen(m_URLs[m_httpSite].m_serverIndex, 128) + 2 : 0) + 8 + m_thumbDataSize + 5 + 16 * m_taskData.m_numTags + 11;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 50, 6);
    buffer->writeString(m_taskData.m_fileName, 128 + 1);
    buffer->writeUInt16(m_taskData.m_fileSlot);
    buffer->writeUInt16(m_URLs[m_httpSite].m_serverType);
    buffer->writeString(m_URLs[m_httpSite].m_serverIndex, 128 + 1);
    buffer->writeUInt32(m_taskData.m_fileSize);
    buffer->writeUInt16(m_taskData.m_category);
    buffer->writeBlob(m_thumbData, m_thumbDataSize);
    buffer->writeArrayStart(10, 2 * m_taskData.m_numTags, 8u);
    for (bdUInt i = 0; i < m_taskData.m_numTags; ++i)
    {
        buffer->writeUInt64(m_taskData.m_tags[i].m_priTag);
        buffer->writeUInt64(m_taskData.m_tags[i].m_secTag);
    }
    buffer->writeArrayEnd();
    m_remoteTaskManager->startTask(task, buffer);
    task->setTaskResult(m_uploadFileID, 1u);
    return task;
}

bdRemoteTaskRef bdContentStreaming::_postUploadSummary()
{
    bdRemoteTaskRef task;
    bdUInt taskSize = 78;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));

    m_remoteTaskManager->initTaskBuffer(buffer, 50, 18);
    buffer->writeUInt64(m_taskData.m_fileID);
    buffer->writeUInt32(m_taskData.m_fileSize);
    m_remoteTaskManager->startTask(task, buffer);
    return task;
}

bdRemoteTaskRef bdContentStreaming::_preCopy(const bdUInt64 fileID, const bdUByte8 taskID)
{
    bdRemoteTaskRef task;
    bdUInt taskSize = 76;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));

    m_remoteTaskManager->initTaskBuffer(buffer, 50, taskID);
    buffer->writeUInt64(fileID);
    buffer->writeUInt16(m_taskData.m_fileSlot);
    m_remoteTaskManager->startTask(task, buffer);
    task->setTaskResult(m_preCopyResults, 3);
    return task;
}

bdRemoteTaskRef bdContentStreaming::_postCopy()
{
    bdRemoteTaskRef task;
    bdUInt copyFileSize = m_httpSite > 1 ? m_preCopyResults[1].m_fileSize : 0;
    bdUInt taskSize = 64;
    taskSize += (m_taskData.m_fileName[0] ? bdStrnlen(m_taskData.m_fileName, 128) + 2 : 0) + 6;
    taskSize += (m_preCopyResults[0].m_source->m_serverIndex[0] ? bdStrnlen(m_preCopyResults[0].m_source->m_serverIndex, 128) + 2 : 0) + 
        13 + m_thumbDataSize + 5 + 16 * m_taskData.m_numTags + 11;

    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, 1));
    m_remoteTaskManager->initTaskBuffer(buffer, 50, 16);
    buffer->writeString(m_taskData.m_fileName, 128 + 1);
    buffer->writeUInt16(m_taskData.m_fileSlot);
    buffer->writeUInt16(m_preCopyResults[0].m_source->m_serverType);
    buffer->writeString(m_preCopyResults[0].m_source->m_serverIndex, 128);
    buffer->writeUInt32(m_preCopyResults[0].m_fileSize);
    buffer->writeUInt16(m_taskData.m_category);
    buffer->writeUInt32(copyFileSize);
    buffer->writeBlob(m_thumbData, m_thumbDataSize);
    buffer->writeArrayStart(10, 2 * m_taskData.m_numTags, 8);
    for (bdUInt i = 0; i < m_taskData.m_numTags; ++i)
    {
        buffer->writeUInt64(m_taskData.m_tags[i].m_priTag);
        buffer->writeUInt64(m_taskData.m_tags[i].m_secTag);
    }
    buffer->readArrayEnd();
    m_remoteTaskManager->startTask(task, buffer);
    task->setTaskResult(m_uploadFileID, 1u);
    return task;
}

bdRemoteTaskRef bdContentStreaming::_preDownloadFileBySlot(bdUInt64 fileID, bdUInt16 fileSlot, bdUInt fileSize, bdFileMetaData* fileMetaData)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdContentStreaming::_preDownloadByFileID(bdUInt64 fileID, bdUInt fileSize, bdFileMetaData* fileMetaData)
{
    bdRemoteTaskRef task;
    bdUInt taskSize = 78;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));

    m_remoteTaskManager->initTaskBuffer(buffer, 50, 9);
    buffer->writeUInt64(fileID);
    buffer->writeUInt32(fileSize);
    m_remoteTaskManager->startTask(task, buffer);
    task->setTaskResult(fileMetaData, 1);
    return task;
}

bdRemoteTaskRef bdContentStreaming::_preDownloadPublisherFile(bdUInt64, bdUInt, bdFileMetaData*)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdContentStreaming::_preDownloadITunesPurchasedFile(bdUInt64, bdUInt, void*, const bdUInt, bdFileMetaData*)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdContentStreaming::_preDownloadSummary()
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdContentStreaming::_preDeleteFile(bdUInt16 fileSlot, bdURL* deleteURLs)
{
    bdRemoteTaskRef task;
    bdUInt taskSize = 67;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));

    m_remoteTaskManager->initTaskBuffer(buffer, 50, 8);
    buffer->writeUInt16(fileSlot);
    m_remoteTaskManager->startTask(task, buffer);
    task->setTaskResult(deleteURLs, 2);
    return task;
}
