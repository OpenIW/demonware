// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdSessionInvite::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdSessionInvite::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdSessionInvite::bdSessionInvite()
    : bdTaskResult(), m_senderID(0), m_sessionID(), m_attachmentSize(0)
{
}

bdSessionInvite::~bdSessionInvite()
{
}

bdBool bdSessionInvite::deserialize(bdByteBufferRef buffer)
{
    bdBool ok = buffer->readUInt64(&m_senderID);
    ok = ok == buffer->readString(m_senderName, sizeof(m_senderName));
    bdUInt sessionIDSize = 8;
    ok = ok == buffer->readBlob(m_sessionID.m_sessionID.ab, &sessionIDSize);
    m_attachmentSize = 1024;
    ok = ok == buffer->readBlob(m_attachment, &m_attachmentSize);
    if (!ok)
    {
        bdLogError("session invite", "Deserialization failed");
    }
    return ok;
}

bdUInt bdSessionInvite::sizeOf()
{
    return sizeof(bdSessionInvite);
}
