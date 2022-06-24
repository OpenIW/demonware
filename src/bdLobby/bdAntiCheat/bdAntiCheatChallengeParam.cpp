// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void* bdAntiCheatChallengeParam::operator new(bdUWord nbytes, void* p)
{
    return p;
}

void* bdAntiCheatChallengeParam::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

void bdAntiCheatChallengeParam::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

bdAntiCheatChallengeParam::bdAntiCheatChallengeParam(const bdAntiCheatChallengeParam& other)
    : m_size(other.m_size), m_data(bdAllocate<bdUByte8>(m_size))
{
    bdMemcpy(m_data, other.m_data, m_size);
}

bdAntiCheatChallengeParam::bdAntiCheatChallengeParam(const bdUByte8* data, bdUInt32 size)
    : m_size(size), m_data(bdAllocate<bdUByte8>(m_size))
{
    bdMemcpy(m_data, data, m_size);
}

bdAntiCheatChallengeParam::bdAntiCheatChallengeParam()
    : m_data(NULL), m_size(0)
{
}

bdAntiCheatChallengeParam::~bdAntiCheatChallengeParam()
{
    if (m_data)
    {
        bdDeallocate<bdUByte8>(m_data);
    }
}

bdBool bdAntiCheatChallengeParam::deserialize(bdByteBufferRef buffer)
{
    return bdBool();
}

bdUInt bdAntiCheatChallengeParam::sizeOf()
{
    return bdUInt();
}
