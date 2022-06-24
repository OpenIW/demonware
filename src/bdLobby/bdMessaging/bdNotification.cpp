// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdNotification::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdNotification::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdNotification::bdNotification()
    : bdTaskResult()
{
}

bdNotification::~bdNotification()
{
}

bdBool bdNotification::deserialize(bdByteBufferRef buffer)
{
    bdUInt type = 0;
    bdBool ok = buffer->readUInt32(type);
    m_type = static_cast<bdNotificationType>(type);
    ok = ok == buffer->readUInt32(m_timeStamp);
    ok = ok == buffer->readUInt64(m_ID);
    ok = ok == buffer->readUInt64(m_sourceUser);
    ok = ok == buffer->readString(m_sourceUserName, sizeof(m_sourceUserName));
    ok = ok == buffer->readUInt64(m_affectedUser);
    ok = ok == buffer->readString(m_affectedUserName, sizeof(m_affectedUserName));
    ok = ok == buffer->readUInt64(m_teamID);
    ok = ok == buffer->readString(m_teamName, sizeof(m_teamName));
    return ok;
}

bdUInt bdNotification::sizeOf()
{
    return sizeof(bdNotification);
}
