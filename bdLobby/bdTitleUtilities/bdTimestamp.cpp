// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdTimeStamp::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdTimeStamp::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdTimeStamp::bdTimeStamp()
    : bdTaskResult(), m_timeStamp(0)
{
}

bdTimeStamp::~bdTimeStamp()
{
}

bdBool bdTimeStamp::deserialize(bdByteBufferRef buffer)
{
    return buffer->readUInt32(&m_timeStamp);
}

bdUInt bdTimeStamp::sizeOf()
{
    return sizeof(bdTimeStamp);
}
