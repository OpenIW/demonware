// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdPooledStorage::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdPooledStorage::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdPooledStorage::bdPooledStorage(bdRemoteTaskManager* const remoteTaskManager)
    : bdContentStreamingBase(remoteTaskManager)
{
}

bdPooledStorage::~bdPooledStorage()
{
}

bdRemoteTaskRef bdPooledStorage::remove(bdUInt64 fileID)
{
    return &bdRemoteTaskRef();
}

bdRemoteTaskRef bdPooledStorage::upload(bdUploadInterceptor* uploadHandle, bdUInt16 category, const bdNChar8* fileName, bdUInt numTags, bdTag* tags, bdFileID* fileID)
{
    if (!initUpload(0, NULL, uploadHandle, 0, fileName, category, NULL, 0, numTags, tags, fileID, 0, false))
    {
        return &bdRemoteTaskRef();
    }
    m_remoteTask = &_preUpload();
    if (m_remoteTask->getStatus() == bdRemoteTask::BD_PENDING)
    {
        return &startUpload();
    }
    else
    {
        return bdRemoteTaskRef(&m_remoteTask);
    }
}

bdRemoteTaskRef bdPooledStorage::download(bdUInt64 fileID, bdDownloadInterceptor* downloadHandler, bdFileMetaData* pooledMetaData, bdUInt startByte, bdUInt endByte)
{
    if (!initDownload(NULL, 0, downloadHandler, pooledMetaData, startByte, endByte))
    {
        return &bdRemoteTaskRef();
    }
    m_taskData.m_fileID = fileID;
    m_remoteTask = &_preDownload();
    if (m_remoteTask->getStatus() == bdRemoteTask::BD_PENDING)
    {
        return &startDownload();
    }
    else
    {
        return bdRemoteTaskRef(&m_remoteTask);
    }
}

bdRemoteTaskRef bdPooledStorage::uploadSummaryMetaData(bdUInt64 fileID, const void* summaryData, bdUInt summaryDataSize, const void* metaData, bdUInt metaDataSize, bdUInt numTags, bdTag* tags)
{
    if (!initUpload(0, summaryData, NULL, summaryDataSize, NULL, 0, metaData, metaDataSize, numTags, tags, NULL, 0, true))
    {
        return &bdRemoteTaskRef();
    }
    m_taskData.m_fileID = fileID;
    m_remoteTask = &_preUploadSummary();
    if (m_remoteTask->getStatus() == bdRemoteTask::BD_PENDING)
    {
        return &startUpload();
    }
    else
    {
        return bdRemoteTaskRef(&m_remoteTask);
    }
}

bdRemoteTaskRef bdPooledStorage::downloadSummary(bdUInt64 fileID, void* summaryData, bdUInt summaryDataSize, bdUInt startByte, bdUInt endByte)
{
    if (!initDownload(summaryData, summaryDataSize, NULL, &m_summaryMeta, startByte, endByte))
    {
        return &bdRemoteTaskRef();
    }
    m_taskData.m_fileID = fileID;
    m_remoteTask = &_preDownloadSummary();
    if (m_remoteTask->getStatus() == bdRemoteTask::BD_PENDING)
    {
        return &startDownload();
    }
    else
    {
        return bdRemoteTaskRef(&m_remoteTask);
    }
}

bdRemoteTaskRef bdPooledStorage::getPooledMetaDataByID(const bdUInt numFiles, const bdUInt64* fileIDs, bdFileMetaData* fileMetas)
{
    bdRemoteTaskRef task;
    bdUInt taskSize = 8 * numFiles + 75;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));

    m_remoteTaskManager->initTaskBuffer(&buffer, 58, 1);
    buffer->writeArrayStart(10, numFiles, 8);
    for (bdUInt i = 0; i < numFiles; ++i)
    {
        buffer->writeUInt64(fileIDs[i]);
    }
    buffer->writeArrayEnd();
    m_remoteTaskManager->startTask(&task, &buffer);
    task->setTaskResult(fileMetas, numFiles);
    return &task;
}

bdRemoteTaskRef bdPooledStorage::_postUploadFile()
{
    bdRemoteTaskRef task;
    bdUInt taskSize = 76;
    bdUInt serverIndexLen;
    if (m_URLs[m_httpSite].m_serverIndex == 0)
    {
        serverIndexLen = 0;
    }
    else
    {
        serverIndexLen = bdStrnlen(m_URLs[m_httpSite].m_serverIndex, 128) + 2;
    }
    taskSize += serverIndexLen + 5;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));

    m_remoteTaskManager->initTaskBuffer(&buffer, 58, 6);
    buffer->writeUInt64(m_taskData.m_fileID);
    buffer->writeUInt16(m_URLs[m_httpSite].m_serverType);
    buffer->writeString(m_URLs[m_httpSite].m_serverIndex, 128 + 1);
    buffer->writeUInt32(m_taskData.m_fileSize);
    m_remoteTaskManager->startTask(&task, &buffer);
    task->setTaskResult(m_uploadFileID, 1);
    return &task;
}

bdRemoteTaskRef bdPooledStorage::_postUploadSummary()
{
    bdRemoteTaskRef task;
    bdUInt taskSize = 78;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));

    m_remoteTaskManager->initTaskBuffer(&buffer, 58, 18);
    buffer->writeUInt64(m_taskData.m_fileID);
    buffer->writeUInt32(m_taskData.m_fileSize);
    m_remoteTaskManager->startTask(&task, &buffer);
    return &task;
}

bdRemoteTaskRef bdPooledStorage::_preUpload()
{
    bdRemoteTaskRef task;
    bdUInt fileNameLen;
    if (m_taskData.m_fileName == 0)
    {
        fileNameLen = 0;
    }
    else
    {
        fileNameLen = bdStrnlen(m_taskData.m_fileName, 128) + 2;
    }
    bdUInt taskSize = fileNameLen + (16 * m_taskData.m_numTags) + 78;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));

    m_remoteTaskManager->initTaskBuffer(&buffer, 58, 5);
    buffer->writeString(m_taskData.m_fileName, 128 + 1);
    buffer->writeUInt16(m_taskData.m_category);
    buffer->writeArrayStart(10, 2 * m_taskData.m_numTags, 8u);
    for (bdUInt i = 0; i < m_taskData.m_numTags; ++i)
    {
        buffer->writeUInt64(m_taskData.m_tags[i].m_priTag);
        buffer->writeUInt64(m_taskData.m_tags[i].m_secTag);
    }
    buffer->writeArrayEnd();
    m_remoteTaskManager->startTask(&task, &buffer);
    task->setTaskResult(m_URLs, 3);
    return &task;
}

bdRemoteTaskRef bdPooledStorage::_preDownload()
{
    bdRemoteTaskRef task;
    bdUInt taskSize = 73;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));

    m_remoteTaskManager->initTaskBuffer(&buffer, 58, 9);
    buffer->writeUInt64(m_taskData.m_fileID);
    m_remoteTaskManager->startTask(&task, &buffer);
    task->setTaskResult(m_downloadMetaData, 1);
    return &task;
}

bdRemoteTaskRef bdPooledStorage::_preUploadSummary()
{
    bdRemoteTaskRef task;
    bdUInt taskSize = m_thumbDataSize + (16 * m_taskData.m_numTags) + 94;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));

    m_remoteTaskManager->initTaskBuffer(&buffer, 58, 17);
    buffer->writeUInt64(m_taskData.m_fileID);
    buffer->writeUInt32(m_taskData.m_fileSize);
    buffer->writeBlob(m_thumbData, m_thumbDataSize);
    buffer->writeArrayStart(10, 2 * m_taskData.m_numTags, 8);
    for (bdUInt i = 0; i < m_taskData.m_numTags; ++i)
    {
        buffer->writeUInt64(m_taskData.m_tags[i].m_priTag);
        buffer->writeUInt64(m_taskData.m_tags[i].m_secTag);
    }
    buffer->writeArrayEnd();
    if (m_sendChecksum)
    {
        buffer->writeBlob(m_checksum, 32);
    }
    m_remoteTaskManager->startTask(&task, &buffer);
    task->setTaskResult(m_URLs, 3);
    return &task;
}

bdRemoteTaskRef bdPooledStorage::_preDownloadSummary()
{
    bdRemoteTaskRef task;
    bdUInt taskSize = 73;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));

    m_remoteTaskManager->initTaskBuffer(&buffer, 58, 19);
    buffer->writeUInt64(m_taskData.m_fileID);
    m_remoteTaskManager->startTask(&task, &buffer);
    task->setTaskResult(&m_summaryMeta, 1);
    return &task;
}

bdUInt bdPooledStorage::getMaxMetaDataSize()
{
    return 512;
}
