// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdAntiCheatChallenge
{
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdAntiCheatChallenge(bdAntiCheatChallenge* other);
    bdAntiCheatChallenge();
    ~bdAntiCheatChallenge();
    bdBool deserialize(bdByteBufferRef buffer);
    bdAntiCheatChallenge* operator=(bdAntiCheatChallenge* other);

    bdUInt16 m_functionID;
    bdUInt64 m_challengeID;
    bdArray<bdAntiCheatChallengeParam> m_parameters;
};