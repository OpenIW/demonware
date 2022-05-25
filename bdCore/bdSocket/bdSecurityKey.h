// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdSecurityKey
{
public:
    bdUByte8 ab[16];

    bdSecurityKey();
    bdSecurityKey(const bdSecurityKey& other);
    bdBool operator==(const bdSecurityKey& other);
};
