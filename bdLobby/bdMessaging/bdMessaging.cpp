// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdMessaging::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdMessaging::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdMessaging::bdMessaging(bdRemoteTaskManager* remoteTaskManager)
    : m_remoteTaskManager(remoteTaskManager)
{
}

bdRemoteTaskRef bdMessaging::getMailHeaders(bdMailHeader*, const unsigned int, const unsigned int)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdMessaging::getMails(unsigned long long*, const unsigned int, bdMailBody*, const bool)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdMessaging::deleteMail(unsigned long long*, const unsigned int)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdMessaging::sendMail(const void*, const unsigned int, unsigned long long*, const unsigned int)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdMessaging::getTeamMailHeaders(const unsigned long long, bdMailHeader*, const unsigned int, const unsigned int)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdMessaging::getTeamMails(const unsigned long long, unsigned long long*, const unsigned int, bdMailBody*, const bool)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdMessaging::deleteTeamMail(const unsigned long long, unsigned long long*, const unsigned int)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdMessaging::sendTeamMail(const void*, const unsigned int, const unsigned long long)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdMessaging::getNotifications(bdNotification* notifications, const bdUInt startIndex, const bdUInt numNotifications, const bdBool deleteOnRead)
{
    bdRemoteTaskRef task;
    bdUInt taskSize = 76;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));

    m_remoteTaskManager->initTaskBuffer(buffer, 6, 5);
    bdBool status = buffer->writeUInt32(startIndex);
    status = status == buffer->writeUInt32(numNotifications);
    status = status == buffer->writeBool(deleteOnRead);
    if (status)
    {
        bdInt startTaskResult = m_remoteTaskManager->startTask(task, buffer);
        if (startTaskResult)
        {
            bdLogWarn("messaging", "Failed to start task: Error %i");
        }
        else
        {
            task->setTaskResult(notifications, numNotifications);
        }
    }
    else
    {
        bdLogWarn("messaging", "Failed to write param into buffer");
    }
    return task;
}

bdRemoteTaskRef bdMessaging::deleteNotifications(unsigned long long*, const unsigned int)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdMessaging::createChatChannel(const bdChannelInfo*)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdMessaging::subscribeToChannel(const unsigned long long)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdMessaging::unsubscribeFromChannel(const unsigned long long)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdMessaging::getChannelList(bdChannelInfo*, const unsigned int)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdMessaging::sendToChannel(const unsigned long long, const void*, const unsigned int)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdMessaging::whisperToChannelMember(const unsigned long long, const unsigned long long, const void*, const unsigned int)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdMessaging::getChannelMembers(const unsigned long long, bdChannelMember*, const unsigned int)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdMessaging::setBlockStatus(const unsigned long long, const bool)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdMessaging::getBlockedList(bdBlockedUser*, const unsigned int, const unsigned int)
{
    return bdRemoteTaskRef();
}

bdRemoteTaskRef bdMessaging::sendGlobalInstantMessage(const bdUInt64 recipientUID, const void* const message, const bdUInt msgSize)
{
    bdRemoteTaskRef task;
    bdUInt newMsgSize = msgSize >= 0x4000 ? 0x4000 : msgSize;
    if (newMsgSize < msgSize)
    {
        bdLogError("messaging", "Exceeded maximum global instant message size, data will be truncated.");
    }
    bdUInt taskSize = newMsgSize + 78;
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));

    m_remoteTaskManager->initTaskBuffer(buffer, 6, 14);
    bdBool status = buffer->writeUInt64(recipientUID);
    status = status == buffer->writeBlob(message, newMsgSize);
    if (status)
    {
        bdInt startTaskResult = m_remoteTaskManager->startTask(task, buffer);
        if (startTaskResult)
        {
            bdLogWarn("messaging", "Failed to start task: Error %i");
        }
    }
    else
    {
        bdLogWarn("messaging", "Failed to write param into buffer");
    }
    return task;
}
