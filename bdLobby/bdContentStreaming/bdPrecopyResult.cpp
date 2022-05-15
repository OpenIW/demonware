// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdPreCopyResult::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

bdPreCopyResult::bdPreCopyResult()
    : bdTaskResult(), m_source(NULL)
{
    reset();
}

bdPreCopyResult::~bdPreCopyResult()
{
}

bdBool bdPreCopyResult::deserialize(bdByteBufferRef buffer)
{
    reset();
    bdBool ok = m_source->deserialize(&bdByteBufferRef(*buffer));
    ok = ok == buffer->readString(m_destination, sizeof(m_destination));
    ok = ok == buffer->readUInt32(&m_fileSize);
    return ok;
}

bdUInt bdPreCopyResult::sizeOf()
{
    return sizeof(bdPreCopyResult);
}

void bdPreCopyResult::reset()
{
    m_destination[0] = 0;
    m_fileSize = 0;
}
