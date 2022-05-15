// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdAntiCheatChallenge::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdAntiCheatChallenge::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdAntiCheatChallenge::bdAntiCheatChallenge(bdAntiCheatChallenge* other)
    : m_functionID(other->m_functionID), m_challengeID(other->m_challengeID), m_parameters(&other->m_parameters)
{
}

bdAntiCheatChallenge::bdAntiCheatChallenge()
    : m_functionID(0), m_challengeID(0), m_parameters(0u)
{
}

bdAntiCheatChallenge::~bdAntiCheatChallenge()
{
}

bdBool bdAntiCheatChallenge::deserialize(bdByteBufferRef buffer)
{
    return bdBool();
}

bdAntiCheatChallenge* bdAntiCheatChallenge::operator=(bdAntiCheatChallenge* other)
{
    return NULL;
}
