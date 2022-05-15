// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdAntiCheatChallenges : public bdReferencable
{
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdAntiCheatChallenges(bdAntiCheatChallenges* other);
    bdAntiCheatChallenges();
    ~bdAntiCheatChallenges();
    bdBool deserialize(bdByteBufferRef buffer);
    bdAntiCheatChallenge* getChallengeByIndex(bdUInt index);
    bdAntiCheatChallenges* operator=(bdAntiCheatChallenges* other);
    bdUInt getNumChallenges();
private:
    bdArray<bdAntiCheatChallenge>* m_challenges;
};