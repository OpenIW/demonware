// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdTags::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdTags::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdTags::bdTags(bdRemoteTaskManager* const remoteTaskManager)
    : m_remoteTaskManager(remoteTaskManager)
{
}

bdTags::~bdTags()
{
    m_remoteTaskManager = NULL;
}

bdRemoteTaskRef bdTags::getTagsForEntityIDs(const bdUInt32 collectionID, const bdUInt32 numEntityIDs, const bdUInt64* entityIDs, bdTagsArray* tagResults)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdTags::setTagsForEntityID(const bdUInt32 collectionID, const bdUInt64 entityID, const bdUInt numTags, const bdTag* tags)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdTags::removeTagsForEntityID(const bdUInt32 collectionID, const bdUInt64 entityID, const bdUInt numTags, const bdTag* tags)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdTags::removeAllTagsForEntityID(const bdUInt32 collectionID, const bdUInt64 entityID)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdTags::searchByTags(const bdUInt32 collectionID, const bdUInt startIndex, const bdUInt maxNumResults, const bdBool orderNewestFirst, const bdUInt numTags, const bdTag* tags, bdFileID* results)
{
    bdRemoteTaskRef task;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(16 * numTags + 92, true));
    bdRemoteTaskManager::initTaskBuffer(&buffer, 52, 5);
    buffer->writeUInt32(collectionID);
    buffer->writeUInt32(startIndex);
    buffer->writeUInt32(maxNumResults);
    buffer->writeBool(orderNewestFirst);
    buffer->writeArrayStart(10, 2 * numTags, 8);
    for (bdUInt i = 0; i < numTags; ++i)
    {
        tags[i].serialize(*buffer);
    }
    buffer->writeArrayEnd();
    m_remoteTaskManager->startTask(&task, &buffer);
    task->setTaskResult(results, maxNumResults);
    return task;
}
