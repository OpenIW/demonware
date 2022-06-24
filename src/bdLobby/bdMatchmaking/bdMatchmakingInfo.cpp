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
    : bdTaskResult(), m_sessionID(), m_hostAddrSize(0), m_gameType(0), m_maxPlayers(0), m_numPlayers(0)
{
    bdMemset(m_hostAddr, 0, sizeof(m_hostAddr));
}

bdMatchMakingInfo::~bdMatchMakingInfo()
{
}

bdBool bdMatchMakingInfo::deserialize(bdByteBufferRef buffer)
{
    bdUInt size = 255;
    bdBool ok = buffer->readBlob(m_hostAddr, size);
    m_hostAddrSize = size;
    size = 8;
    ok = ok == buffer->readBlob(m_sessionID.ab, size);
    ok = ok == buffer->readUInt32(m_gameType);
    ok = ok == buffer->readUInt32(m_maxPlayers);
    ok = ok == buffer->readUInt32(m_numPlayers);
    if (!ok)
    {
        bdLogError("match making info", "Deserialization failed");
    }
    return ok;
}

bdUInt bdMatchMakingInfo::sizeOf()
{
    return sizeof(bdMatchMakingInfo);
}

void bdMatchMakingInfo::serialize(bdByteBuffer& buffer)
{
    buffer.writeBlob(m_hostAddr, m_hostAddrSize);
    buffer.writeUInt32(m_gameType);
    buffer.writeUInt32(m_maxPlayers);
}

bdCommonAddrRef bdMatchMakingInfo::getHostAddrAsCommonAddr(bdCommonAddrRef localCommonAddr)
{
    bdCommonAddrRef newAddr;
    if (m_hostAddrSize != BD_COMMON_ADDR_SERIALIZED_SIZE)
    {
        bdLogError("match making info", "m_hostAddrSize != BD_COMMON_ADDR_SERIALIZED_SIZE. ");
        return newAddr;
    }
    newAddr = new bdCommonAddr();
    newAddr->deserialize(bdCommonAddrRef(localCommonAddr), m_hostAddr);
    if (!*localCommonAddr)
    {
        bdLogWarn("match making info", "localCommonAddr not set. ");
    }
    return newAddr;
}

void bdMatchMakingInfo::setHostAddr(bdCommonAddrRef localCommonAddr)
{
    if (localCommonAddr.notNull())
    {
        localCommonAddr->serialize(m_hostAddr);
        m_hostAddrSize = BD_COMMON_ADDR_SERIALIZED_SIZE;
    }
    else
    {
        bdMemset(m_hostAddr, 0, sizeof(m_hostAddr));
        m_hostAddrSize = 0;
    }
}
