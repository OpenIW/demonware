// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdStatsInfo::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdStatsInfo::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

void* bdStatsInfo::operator new(bdUWord nbytes, void* p)
{
    return p;
}

bdStatsInfo::bdStatsInfo()
    : bdTaskResult(), m_leaderboardID(0), m_entityID(0), m_writeType(bdStats::STAT_WRITE_ADD), m_rating(0), m_rank(0), m_secondsSinceUpdate(0)
{
    bdMemset(m_entityName, 0, sizeof(m_entityName));
}

bdStatsInfo::~bdStatsInfo()
{
}

bdBool bdStatsInfo::deserialize(bdByteBufferRef buffer)
{
    bdBool ok = buffer->readUInt64(&m_entityID);
    ok = ok == buffer->readInt64(&m_rating);
    ok = ok == buffer->readUInt64(&m_rank);
    ok = ok == buffer->readString(m_entityName, sizeof(m_entityName));
    ok = ok == buffer->readUInt32(&m_secondsSinceUpdate);
    if (!ok)
    {
        bdLogError("statsInfo", "Deserialization failed");
    }
    return ok;
}

void bdStatsInfo::serialize(bdByteBuffer* buffer)
{
    buffer->writeUInt32(m_leaderboardID);
    buffer->writeUInt64(m_entityID);
    buffer->writeUByte8(m_writeType);
    buffer->writeInt64(m_rating);
}

bdUInt bdStatsInfo::sizeOf()
{
    return sizeof(bdStatsInfo);
}
