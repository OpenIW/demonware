// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdAuthTicket
{
public:
    enum bdTicketTypes
    {
        BD_AUTH_USER_TO_SERVICE_TICKET = 0,
        BD_AUTH_HOST_TO_SERVICE_TICKET = 1,
        BD_AUTH_USER_TO_HOST_TICKET = 2
    };

    bdUInt m_magicNumber;
    bdUByte8 m_type;
    bdUInt m_titleID;
    bdUInt m_timeIssued;
    bdUInt m_timeExpires;
    bdUInt64 m_licenseID;
    bdUInt64 m_userID;
    bdNChar8 m_username[64];
    bdUByte8 m_sessionKey[24];
private:
    bdUByte8 m_usingHashMagicNumber[3];
    bdUByte8 m_hash[4];
public:
    bdAuthTicket();
    void deserialize(const void* buffer);
    void serialize(void* buffer);
};