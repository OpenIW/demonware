// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdRandom
{
protected:
    bdUInt m_val;
public:
    bdRandom();
    bdUInt nextUInt();
    void nextUBytes(bdUByte8* in, const bdInt length);
    void setSeed(bdUInt seed);
};
