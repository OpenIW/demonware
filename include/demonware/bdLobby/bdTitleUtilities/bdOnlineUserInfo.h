// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdOnlineUserInfo : bdTaskResult
{
public:
    bdUInt64 m_userID;
    bdBool m_isOnline;
    
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdOnlineUserInfo();
    ~bdOnlineUserInfo();
};
