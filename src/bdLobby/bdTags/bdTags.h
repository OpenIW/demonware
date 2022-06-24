// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdTagsArray : public bdTaskResult
{
public:
    bdUInt64 m_entityID;
    bdUInt32 m_numTags;
    bdTag m_tags[60];
};

class bdTags
{
protected:
    bdRemoteTaskManager* m_remoteTaskManager;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdTags(bdRemoteTaskManager* remoteTaskManager);
    ~bdTags();
    bdRemoteTaskRef getTagsForEntityIDs(const bdUInt32 collectionID, const bdUInt32 numEntityIDs, const bdUInt64 *entityIDs, bdTagsArray *tagResults);
    bdRemoteTaskRef setTagsForEntityID(const bdUInt32 collectionID, const bdUInt64 entityID, const bdUInt numTags, const bdTag* tags);
    bdRemoteTaskRef removeTagsForEntityID(const bdUInt32 collectionID, const bdUInt64 entityID, const bdUInt numTags, const bdTag* tags);
    bdRemoteTaskRef removeAllTagsForEntityID(const bdUInt32 collectionID, const bdUInt64 entityID);
    bdRemoteTaskRef searchByTags(const bdUInt32 collectionID, const bdUInt startIndex, const bdUInt maxNumResults, const bdBool orderNewestFirst,
        const bdUInt numTags, const bdTag* tags, bdFileID* results);
};
