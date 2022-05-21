// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdVoteRankStatsInfo::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdVoteRankStatsInfo::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdVoteRankStatsInfo::bdVoteRankStatsInfo()
    : bdStatsInfo(), m_fileOwnerID(0), m_totalVotes(0), m_avgVoteValue(0)
{
    bdMemset(m_fileOwnerName, 0, sizeof(m_fileOwnerName));
}

bdVoteRankStatsInfo::~bdVoteRankStatsInfo()
{
}

void bdVoteRankStatsInfo::serialize(bdByteBuffer* buffer)
{
    bdStatsInfo::serialize(buffer);
    buffer->writeUInt64(m_fileOwnerID);
    buffer->writeString(m_fileOwnerName, sizeof(m_fileOwnerName) + 1);
    buffer->writeUInt32(m_avgVoteValue);
    buffer->writeUInt32(m_totalVotes);
}

bdBool bdVoteRankStatsInfo::deserialize(bdByteBufferRef buffer)
{
    bdBool ok = bdStatsInfo::deserialize(bdByteBufferRef(*buffer));
    ok = ok == buffer->readUInt64(&m_fileOwnerID);
    ok = ok == buffer->readString(m_fileOwnerName, sizeof(m_fileOwnerName) + 1);
    ok = ok == buffer->readUInt32(&m_avgVoteValue);
    ok = ok == buffer->readUInt32(&m_totalVotes);
    return ok;
}

bdUInt bdVoteRankStatsInfo::sizeOf()
{
    return sizeof(bdVoteRankStatsInfo);
}
