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
