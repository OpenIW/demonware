// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdRatingInfo::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdRatingInfo::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdRatingInfo::bdRatingInfo()
    : bdTaskResult(), m_entityID(0), m_rating(0)
{
}

void bdRatingInfo::serialize(bdByteBuffer* buffer)
{
    buffer->writeUInt64(m_entityID);
    buffer->writeUByte8(m_rating);
}

bdBool bdRatingInfo::deserialize(bdByteBufferRef buffer)
{
    return true;
}

bdUInt bdRatingInfo::sizeOf()
{
    return sizeof(bdRatingInfo);
}
