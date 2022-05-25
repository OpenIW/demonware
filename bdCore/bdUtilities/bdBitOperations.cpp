// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdCore/bdCore.h"

bdUInt bdBitOperations::nextPowerOf2(const bdUInt v)
{
    bdUInt result;

    result = ~((((((((((v | (v >> 1)) >> 2) | v | (v >> 1)) >> 4) | ((v | (v >> 1)) >> 2) | v | (v >> 1)) >> 8) | ((((v | (v >> 1)) >> 2) | v | (v >> 1)) >> 4) | ((v | (v >> 1)) >> 2) | v | (v >> 1)) >> 16) | ((((((v | (v >> 1)) >> 2) | v | (v >> 1)) >> 4) | ((v | (v >> 1)) >> 2) | v | (v >> 1)) >> 8) | ((((v | (v >> 1)) >> 2) | v | (v >> 1)) >> 4) | ((v | (v >> 1)) >> 2) | v | (v >> 1)) >> 1) & (((((((((v | (v >> 1)) >> 2) | v | (v >> 1)) >> 4) | ((v | (v >> 1)) >> 2) | v | (v >> 1)) >> 8) | ((((v | (v >> 1)) >> 2) | v | (v >> 1)) >> 4) | ((v | (v >> 1)) >> 2) | v | (v >> 1)) >> 16) | ((((((v | (v >> 1)) >> 2) | v | (v >> 1)) >> 4) | ((v | (v >> 1)) >> 2) | v | (v >> 1)) >> 8) | ((((v | (v >> 1)) >> 2) | v | (v >> 1)) >> 4) | ((v | (v >> 1)) >> 2) | v | (v >> 1));
    if (result != v)
    {
        result *= 2;
    }
    return result;
}

bdUInt bdBitOperations::highBitNumber(bdUInt v)
{
    bdInt i;

    i = 16;
    if ((v & 0xFFFF0000) == 0)
    {
        i = 0;
    }
    v >>= i;
    if ((v & 0xFF00) != 0)
    {
        i |= 8u;
        v >>= 8;
    }
    if ((v & 0xF0) != 0)
    {
        i |= 4u;
        v >>= 4;
    }
    if ((v & 0xC) != 0)
    {
        i |= 2u;
        v >>= 2;
    }
    return (v >> 1) | i;
}
