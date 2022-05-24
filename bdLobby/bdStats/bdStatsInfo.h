// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdStatsInfo : public bdTaskResult
{
public:
    bdUInt m_leaderboardID;
    bdUInt64 m_entityID;
    bdStats::bdWriteType m_writeType;
    bdInt64 m_rating;
    bdUInt64 m_rank;
    bdNChar8 m_entityName[65];
    bdUInt m_secondsSinceUpdate;

    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    void* operator new(bdUWord nbytes, void* p);
    bdStatsInfo();
    ~bdStatsInfo();
    virtual bdBool deserialize(bdByteBufferRef buffer);
    virtual void serialize(bdByteBuffer* buffer);
    virtual bdUInt sizeOf();
};
