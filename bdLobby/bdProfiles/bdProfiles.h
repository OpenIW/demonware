// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdProfiles
{
protected:
    bdRemoteTaskManager* m_remoteTaskManager;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdProfiles(bdRemoteTaskManager* const remoteTaskManager);
    ~bdProfiles();
    bdRemoteTaskRef deleteProfile();
    bdRemoteTaskRef setPublicInfo(bdProfileInfo* profileInfo);
    bdRemoteTaskRef setPrivateInfo(bdProfileInfo* profileInfo);
    bdRemoteTaskRef getPublicInfos(const bdUInt64* userIDs, bdProfileInfo* publicProfiles, const bdUInt numProfiles);
    bdRemoteTaskRef getPrivateInfo(bdProfileInfo* const privateProfile);
};