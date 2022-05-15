// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdAntiCheatChallengeParam
{
public:
    bdUByte8* m_data;
    bdUInt m_size;

    bdAntiCheatChallengeParam(bdAntiCheatChallengeParam* other);
    bdAntiCheatChallengeParam(const bdUByte8* data, bdUInt32 size);
    bdAntiCheatChallengeParam();
    ~bdAntiCheatChallengeParam();
    bdAntiCheatChallengeParam* operator=(const bdAntiCheatChallengeParam* other);
    bdBool operator==(class bdAntiCheatChallengeParam* other);
    bdBool operator!=(class bdAntiCheatChallengeParam* other);
    bdBool deserialize(bdByteBufferRef buffer);
    bdUInt sizeOf();
};