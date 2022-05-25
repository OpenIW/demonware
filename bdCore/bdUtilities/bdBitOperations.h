// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdBitOperations
{
public:
    static bdUInt nextPowerOf2(const bdUInt v);
    static bdUInt highBitNumber(bdUInt v);

    template<typename T>
    static void endianSwap(const T& src, T& dest)
    {
        dest = src;
    };
};
