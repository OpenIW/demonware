// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdAuthUtility
{
public:
    static bdUInt64 getLicenseID(const bdNChar8* licenseCode);
    static void getLicenseKey(const bdNChar8* licenaseCode, bdByte8* licenseKey);
    static void getUserKey(const bdNChar8* password, bdByte8* userKey);
    static bdUInt64 getUserID(const bdNChar8* username);
};

struct bdAuthCreateAccountPlainText
{
    bdUInt64 m_licenseID;

    void serialize(void* buffer, const bdUInt bufferSize);
    void deserialize(const void* buffer, const bdUInt bufferSize);
};

struct bdAuthCreateAccountCypherText
{
    bdUInt32 m_magicNumber;
    bdNChar8 m_username[64];
    bdByte8 m_userKey[24];

    void serialize(void* buffer, const bdUInt bufferSize);
    void deserialize(const void* buffer, const bdUInt bufferSize);
};
