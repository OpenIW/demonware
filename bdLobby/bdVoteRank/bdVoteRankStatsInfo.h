// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#pragma pack(8)
class __declspec(align(8)) bdVoteRankStatsInfo : bdStatsInfo
{
public:
    bdUInt64 m_fileOwnerID;
    bdNChar8 m_fileOwnerName[65];
    bdUInt m_totalVotes;
    bdUInt m_avgVoteValue;

    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdVoteRankStatsInfo();
    ~bdVoteRankStatsInfo();
    virtual void serialize(bdByteBuffer* buffer);
    virtual bdBool deserialize(bdByteBufferRef buffer);
    virtual bdUInt sizeOf();
};
#pragma pop