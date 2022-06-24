// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdMessaging
{
public:
    enum bdBlockLevel : bdInt
    {
        BD_NOT_BLOCKED = 0x0,
        BD_BLOCKED = 0x1,
    };
protected:
    bdRemoteTaskManager* m_remoteTaskManager;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdMessaging(bdRemoteTaskManager* remoteTaskManager);
    ~bdMessaging();
    bdRemoteTaskRef getMailHeaders(class bdMailHeader*, const unsigned int, const unsigned int);
    bdRemoteTaskRef getMails(unsigned long long*, const unsigned int, class bdMailBody*, const bool);
    bdRemoteTaskRef deleteMail(unsigned long long*, const unsigned int);
    bdRemoteTaskRef sendMail(const void*, const unsigned int, unsigned long long*, const unsigned int);
    bdRemoteTaskRef getTeamMailHeaders(const unsigned long long, class bdMailHeader*, const unsigned int, const unsigned int);
    bdRemoteTaskRef getTeamMails(const unsigned long long, unsigned long long*, const unsigned int, class bdMailBody*, const bool);
    bdRemoteTaskRef deleteTeamMail(const unsigned long long, unsigned long long*, const unsigned int);
    bdRemoteTaskRef sendTeamMail(const void*, const unsigned int, const unsigned long long);
    bdRemoteTaskRef getNotifications(bdNotification* notifications, const bdUInt startIndex, const bdUInt numNotifications, const bdBool deleteOnRead);
    bdRemoteTaskRef deleteNotifications(unsigned long long*, const unsigned int);
    bdRemoteTaskRef createChatChannel(const class bdChannelInfo*);
    bdRemoteTaskRef subscribeToChannel(const unsigned long long);
    bdRemoteTaskRef unsubscribeFromChannel(const unsigned long long);
    bdRemoteTaskRef getChannelList(class bdChannelInfo*, const unsigned int);
    bdRemoteTaskRef sendToChannel(const unsigned long long, const void*, const unsigned int);
    bdRemoteTaskRef whisperToChannelMember(const unsigned long long, const unsigned long long, const void*, const unsigned int);
    bdRemoteTaskRef getChannelMembers(const unsigned long long, class bdChannelMember*, const unsigned int);
    bdRemoteTaskRef setBlockStatus(const unsigned long long, const bool);
    bdRemoteTaskRef getBlockedList(class bdBlockedUser*, const unsigned int, const unsigned int);
    bdRemoteTaskRef sendGlobalInstantMessage(const bdUInt64 recipientUID, const void* const message, const bdUInt msgSize);
};
