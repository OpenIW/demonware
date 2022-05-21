// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdVerifyString::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdVerifyString::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdVerifyString::bdVerifyString()
    : bdTaskResult(), m_verified(false)
{
}

bdVerifyString::~bdVerifyString()
{
}

bdBool bdVerifyString::deserialize(bdByteBufferRef buffer)
{
    bdUInt verified = 0;
    bdBool ok = buffer->readUInt32(&verified);
    m_verified = ok && !verified;
    return ok;
}

bdUInt bdVerifyString::sizeOf()
{
    return sizeof(bdVerifyString);
}
