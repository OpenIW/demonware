// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdSessionParams::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdSessionParams::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdSessionParams::~bdSessionParams()
{
}

bdBool bdSessionParams::serialize(bdByteBuffer* buffer)
{
    return true;
}

bdUInt bdSessionParams::sizeOf()
{
    bdLogWarn("bdMatchMaking/bdSessionParams", "sizeOf() should not be used for this class; use serializedSizeOf() instead.");
    return sizeof(bdSessionParams);
}
