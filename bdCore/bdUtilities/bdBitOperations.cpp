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
    int v2; // [esp+0h] [ebp-4h]
    unsigned int v3; // [esp+Ch] [ebp+8h]

    v2 = (v & 0xFFFF0000) != 0 ? 0x10 : 0;
    v3 = v >> ((v & 0xFFFF0000) != 0 ? 0x10 : 0);
    if ((v3 & 0xFF00) != 0)
    {
        v2 |= 8u;
        v3 >>= 8;
    }
    if ((v3 & 0xF0) != 0)
    {
        v2 |= 4u;
        v3 >>= 4;
    }
    if ((v3 & 0xC) != 0)
    {
        v2 |= 2u;
        v3 >>= 2;
    }
    return v2 | (v3 >> 1);
}
