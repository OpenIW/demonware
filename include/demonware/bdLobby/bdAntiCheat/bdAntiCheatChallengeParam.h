// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdAntiCheatChallengeParam
{
public:
    bdUByte8* m_data;
    bdUInt m_size;

    void* operator new(bdUWord nbytes, void* p);
    void* operator new(bdUWord nbytes);
    void operator delete(void* p);
    bdAntiCheatChallengeParam(const bdAntiCheatChallengeParam& other);
    bdAntiCheatChallengeParam(const bdUByte8* data, bdUInt32 size);
    bdAntiCheatChallengeParam();
    ~bdAntiCheatChallengeParam();
    bdBool deserialize(bdByteBufferRef buffer);
    bdUInt sizeOf();
};
