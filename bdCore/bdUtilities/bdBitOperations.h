// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdBitOperations
{
public:
	static unsigned int nextPowerOf2(const unsigned int v);
	static unsigned int highBitNumber(unsigned int v);

	template<typename T>
    static void endianSwap(const T* src, T* dest)
    {
        *dest = *src;
    };
};
