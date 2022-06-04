// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdGroup
{
protected:
    bdRemoteTaskManager* m_remoteTaskManager;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdGroup(bdRemoteTaskManager* remoteTaskManager);
    ~bdGroup();
    bdRemoteTaskRef setGroups(const bdUInt32* const groupIDs, const bdUInt numEntries);
    bdRemoteTaskRef setGroupsForEntity(const bdUInt64 entityID, const bdUInt32* const groupIDs, const bdUInt numEntries);
    bdRemoteTaskRef getEntityGroups(const bdUInt64 entityID, bdGroupID* results, const bdUInt numEntries);
    bdRemoteTaskRef getGroupCounts(const bdUInt32* const groupIDs, const bdUInt numGroupIDs, bdGroupCount* results, const bdUInt numEntries);
};
