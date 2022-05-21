// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdOnlineUserInfo::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdOnlineUserInfo::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdOnlineUserInfo::bdOnlineUserInfo()
    : bdTaskResult(), m_userID(0), m_isOnline(false)
{
}

bdOnlineUserInfo::~bdOnlineUserInfo()
{
}
