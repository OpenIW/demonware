// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdPlatform/bdPlatform.h"

void* bdMemset(void* const s, const bdInt c, const bdUWord len)
{
    return memset(s, c, len);
}

void* bdMemcpy(void* const dest, const void* const src, const bdUWord len)
{
    return memcpy(dest, src, len);
}

bdInt bdMemcmp(const void* const s1, const void* const s2, const bdUWord len)
{
    return memcmp(s1, s2, len);
}

void* bdMemmove(void* const dest, const void* const src, const bdUWord len)
{
    return memmove(dest, src, len);;
}
