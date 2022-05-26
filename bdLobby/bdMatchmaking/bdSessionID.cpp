// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdSessionID::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdSessionID::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdSessionID::bdSessionID()
    : bdTaskResult(), m_sessionID()
{
    bdMemset(m_sessionID.ab, 0, sizeof(m_sessionID.ab));
}

bdSessionID::bdSessionID(const bdSessionID* sessionID)
    : bdTaskResult(), m_sessionID()
{
    bdMemcpy(m_sessionID.ab, sessionID->m_sessionID.ab, sizeof(m_sessionID.ab));
}

bdSessionID::~bdSessionID()
{
}

void bdSessionID::serialize(bdByteBuffer& buffer) const
{
    buffer.writeBlob(m_sessionID.ab, sizeof(m_sessionID.ab));
}

bdBool bdSessionID::deserialize(bdByteBufferRef buffer)
{
    bdUInt readSize = 8;
    return buffer->readBlob(m_sessionID.ab, readSize);
}

bdUInt bdSessionID::sizeOf()
{
    return sizeof(bdSessionID);
}
