// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdSecurityKeyMapListener
{
public:
    virtual ~bdSecurityKeyMapListener() {};
    virtual void onSecurityKeyRemove(const bdSecurityID& secID) {};
};
