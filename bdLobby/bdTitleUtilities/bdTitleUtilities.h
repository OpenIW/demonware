// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdTitleUtilities
{
protected:
    bdRemoteTaskManager* m_remoteTaskManager;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdTitleUtilities(bdRemoteTaskManager* remoteTaskManager);
    bdRemoteTaskRef verifyString(const bdNChar8* const str, const bdUInt length, bdVerifyString* verified);
    bdRemoteTaskRef recordEvent(const bdNChar8* const eventStr, const bdUInt length, bdUInt category);
    bdRemoteTaskRef recordEventBin(const void* eventBlob, const bdUInt length, bdUInt category);
    bdRemoteTaskRef getTitleStats(class bdTitleStats*);
    bdRemoteTaskRef areUsersOnline(class bdOnlineUserInfo*, const unsigned int);
    bdRemoteTaskRef getServerTime(bdTimeStamp* timeStamp);
    bdRemoteTaskRef sendOwnedContent(const void*, const unsigned int);
};
