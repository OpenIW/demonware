// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdMatchMakingInfo::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdMatchMakingInfo::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdMatchMakingInfo::bdMatchMakingInfo()
{
}

bdMatchMakingInfo::~bdMatchMakingInfo()
{
}

bdBool bdMatchMakingInfo::deserialize(bdByteBufferRef buffer)
{
    return bdBool();
}

bdUInt bdMatchMakingInfo::sizeOf()
{
    return bdUInt();
}

void bdMatchMakingInfo::serialize(bdByteBuffer* buffer)
{
}

bdCommonAddrRef bdMatchMakingInfo::getHostAddrAsCommonAddr(bdCommonAddrRef localCommonAddr)
{
    return bdCommonAddrRef();
}

void bdMatchMakingInfo::setHostAddr(bdCommonAddrRef localCommonAddr)
{
}
