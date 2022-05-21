// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

enum bdNotificationType : bdInt
{
    BD_FRIENDSHIP_REJECTED = 0x1,
    BD_FRIENDSHIP_ACCEPTED = 0x2,
    BD_FRIENDSHIP_REVOKED = 0x3,
    BD_TEAMNAME_UPDATED = 0x4,
    BD_TEAMMEMBER_PROMOTED_TO_ADMIN = 0x5,
    BD_TEAMMEMBER_PROMOTED_TO_OWNER = 0x6,
    BD_TEAMMEMBER_DEMOTED_TO_ORDINARY = 0x7,
    BD_TEAMMEMBER_KICKED = 0x8,
    BD_TEAM_DISSOLVED = 0x9,
    BD_TEAMMEMBER_LEFT = 0xA,
    BD_TEAMMEMBER_INVITE_REJECTED = 0xB,
    BD_TEAMMEMBER_INVITE_ACCEPTED = 0xC,
};

class bdNotification : public bdTaskResult
{
public:
    bdNotificationType m_type;
    bdUInt32 m_timeStamp;
    bdUInt64 m_ID;
    bdUInt64 m_sourceUser;
    bdNChar8 m_sourceUserName[64];
    bdUInt64 m_affectedUser;
    bdNChar8 m_affectedUserName[64];
    bdUInt64 m_teamID;
    bdNChar8 m_teamName[64];

    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdNotification();
    ~bdNotification();
    virtual bdBool deserialize(bdByteBufferRef buffer);
    virtual bdUInt sizeOf();
};
