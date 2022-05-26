// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdRemoteTaskManager
{
protected:
    bdLinkedList<bdRemoteTaskRef> m_tasks;
    bdHashMap<bdUInt64, bdRemoteTaskRef, bdHashingClass> m_asyncTasks;
    bdHashMap<bdUInt64, bdByteBufferRef, bdHashingClass> m_asyncResults;
    bdLobbyConnectionRef m_lobbyConnection;
    bdBool m_encryptedConnection;
    bdUInt64 m_connectionID;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdRemoteTaskManager(bdLobbyConnectionRef lobbyConnection, const bdBool useEncryption);
    virtual ~bdRemoteTaskManager();
    void initTaskBuffer(bdTaskByteBufferRef& buffer, const bdUByte8 serviceID, const bdUByte8 taskID);
    bdLobbyErrorCode startTask(bdRemoteTaskRef& newTask, bdTaskByteBufferRef& queryParams);
    bdLobbyErrorCode startLSGTask(bdRemoteTaskRef& newTask, const bdUByte8 serviceID, const bdUByte8 taskID, const void* const queryParams, const bdUInt queryParamsSize);
    bdLobbyErrorCode sendTask(bdRemoteTaskRef newTask, bdTaskByteBufferRef& queryParams);
    bdUInt64 getConnectionID() const;
    void setConnectionID(bdUInt64 connectionID);
    void handleTaskReply(const bdByteBufferRef buffer);
    void handleLSGTaskReply(const bdByteBufferRef buffer);
    void handleAsyncResult(const bdByteBufferRef buffer);
    void cleanUpAsyncState();
};
