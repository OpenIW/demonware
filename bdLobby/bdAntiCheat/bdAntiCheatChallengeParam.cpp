// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

bdAntiCheatChallengeParam::bdAntiCheatChallengeParam(bdAntiCheatChallengeParam* other)
    : m_size(other->m_size), m_data(bdAllocate<bdUByte8>(m_size))
{
    bdMemcpy(m_data, other->m_data, m_size);
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

bdAntiCheatChallengeParam* bdAntiCheatChallengeParam::operator=(const bdAntiCheatChallengeParam* other)
{
    return NULL;
}

bdBool bdAntiCheatChallengeParam::operator==(bdAntiCheatChallengeParam* other)
{
    return bdBool();
}

bdBool bdAntiCheatChallengeParam::operator!=(bdAntiCheatChallengeParam* other)
{
    return bdBool();
}

bdBool bdAntiCheatChallengeParam::deserialize(bdByteBufferRef buffer)
{
    return bdBool();
}

bdUInt bdAntiCheatChallengeParam::sizeOf()
{
    return bdUInt();
}
