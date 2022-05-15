// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdAntiCheatChallenges::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdAntiCheatChallenges::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdAntiCheatChallenges::bdAntiCheatChallenges(bdAntiCheatChallenges* other)
{
}

bdAntiCheatChallenges::bdAntiCheatChallenges()
    : bdReferencable(), m_challenges(NULL)
{
}

bdAntiCheatChallenges::~bdAntiCheatChallenges()
{
}

bdBool bdAntiCheatChallenges::deserialize(bdByteBufferRef buffer)
{
    return bdBool();
}

bdAntiCheatChallenge* bdAntiCheatChallenges::getChallengeByIndex(bdUInt index)
{
    return NULL;
}

bdAntiCheatChallenges* bdAntiCheatChallenges::operator=(bdAntiCheatChallenges* other)
{
    return NULL;
}

bdUInt bdAntiCheatChallenges::getNumChallenges()
{
    return bdUInt();
}
