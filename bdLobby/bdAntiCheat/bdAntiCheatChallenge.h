// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdAntiCheatChallenge
{
public:
    bdUInt16 m_functionID;
    bdUInt64 m_challengeID;
    bdArray<bdAntiCheatChallengeParam> m_parameters;

    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdAntiCheatChallenge(bdAntiCheatChallenge& other);
    bdAntiCheatChallenge();
    ~bdAntiCheatChallenge();
    bdBool deserialize(bdByteBufferRef buffer);
};
