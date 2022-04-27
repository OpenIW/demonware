// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdPlatform/bdPlatform.h"

bdInAddr::bdInAddr()
{
    inUn.m_iaddr = 0xFF00FF00;
}

bdInAddr::bdInAddr(const bdNChar8* addr)
{
    fromString(addr);
}

void bdInAddr::fromString(const char* addr)
{
    inUn.m_iaddr = inet_addr(addr);
}

const bdUInt bdInAddr::toString(char* str, unsigned int size) const
{
    const char* src;

    src = inet_ntoa((in_addr&)*this);
    if (src)
    {
        bdStrlcpy(str, src, size);
        return strlen(str);
    }
    return 0;
}
