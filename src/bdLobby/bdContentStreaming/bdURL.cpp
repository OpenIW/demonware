// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdURL::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

bdURL::bdURL()
    : bdTaskResult(), m_serverType(0), m_fileID(0)
{
    m_url[0] = 0;
    m_serverIndex[0] = 0;
}

bdURL::~bdURL()
{
}

bdBool bdURL::deserialize(bdByteBufferRef buffer)
{
    bdBool ok = buffer->readString(m_url, sizeof(m_url));
    ok = ok == buffer->readUInt16(m_serverType);
    ok = ok == buffer->readString(m_serverIndex, sizeof(m_serverIndex));
    ok = ok == buffer->readUInt64(m_fileID);
    return ok;
}

bdUInt bdURL::sizeOf()
{
    return sizeof(bdURL);
}
