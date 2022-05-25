// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdCore/bdCore.h"

bdRandom::bdRandom()
{
    setSeed(bdPlatformTiming::getHiResTimeStamp().LowPart);
}

bdUInt bdRandom::nextUInt()
{
    bdUInt next;

    next = 0x41A7 * (m_val % 0x1F31D) - 0xB14 * (m_val / 0x1F31D);
    if (next <= 0)
    {
        next += 0x7FFFFFFF;
    }
    m_val = next;
    return m_val;
}

void bdRandom::nextUBytes(bdUByte8* in, const bdInt length)
{
    int i;

    for (i = 0; i < length; ++i)
    {
        in[i] = nextUInt();
    }
}

void bdRandom::setSeed(bdUInt seed)
{
    m_val = seed;
    if (!seed)
    {
        bdLogWarn("random", "Shouldn't use 0 for seed. 12,195,257 used instead.");
        m_val = 12195257;
    }
}
