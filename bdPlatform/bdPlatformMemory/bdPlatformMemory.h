// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

inline void* bdMemset(void* const s, const bdInt c, const bdUWord len)
{
    memset(s, c, len);
    return s;
}

inline void* bdMemcpy(void* const dest, const void* const src, const bdUWord len)
{
    memcpy(dest, src, len);
    return dest;
}

inline bdInt bdMemcmp(const void* const s1, const void* const s2, const bdUWord len)
{
    return memcmp(s1, s2, len);
}

inline void* bdMemmove(void* const dest, const void* const src, const bdUWord len)
{
    memmove(dest, src, len);
    return dest;
}
