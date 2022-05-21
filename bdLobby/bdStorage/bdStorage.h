// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdStorage
{
protected:
    bdRemoteTaskManager* m_remoteTaskManager;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdStorage(bdRemoteTaskManager* remoteTaskManager);
    ~bdStorage();
    bdRemoteTaskRef uploadFile(const bdNChar8* const fileName, const void* const fileData, const bdUInt fileSize, const bdFileInfo::bdVisibility fileVisibility,
        const bdUInt64 ownerID, bdFileInfo* const fileInfo);
    bdRemoteTaskRef removeFile(const bdNChar8* const fileName, const bdUInt64 ownerID);
    bdRemoteTaskRef getFile(const bdNChar8* const fileName, bdFileData* const fileData, const bdUInt64 ownerUID);
    bdRemoteTaskRef getFileByID(const unsigned long long, const class bdFileData*);
    bdRemoteTaskRef getFilesByID(const unsigned int, unsigned long long*, class bdGetFilesResult*);
    bdRemoteTaskRef listFilesByOwner(const unsigned long long, const unsigned int, class bdFileInfo*, const unsigned short, const unsigned short, const char*);
    bdRemoteTaskRef listAllPublisherFiles(const unsigned int, class bdFileInfo*, const unsigned short, const unsigned short, const char*);
    bdRemoteTaskRef updateFile(const unsigned long long, void*, unsigned int);
    bdRemoteTaskRef getPublisherFile(const bdNChar8* const fileName, bdFileData* const fileData);
};
