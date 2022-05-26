// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdProfileInfo::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdProfileInfo::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdProfileInfo::bdProfileInfo()
    : bdTaskResult(), m_entityID(0)
{
}

bdProfileInfo::~bdProfileInfo()
{
}

bdBool bdProfileInfo::deserialize(bdByteBufferRef buffer)
{
    bdBool status = buffer->readUInt64(m_entityID);
    bdAssert(status, "Deserialization failed.");
    return status;
}

bdUInt bdProfileInfo::sizeOf()
{
    return sizeof(bdProfileInfo);
}

void bdProfileInfo::serialize(bdByteBuffer& buffer)
{
    buffer.writeUInt64(m_entityID);
}
