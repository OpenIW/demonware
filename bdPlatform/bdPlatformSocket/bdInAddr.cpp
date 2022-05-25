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

void bdInAddr::fromString(const bdNChar8* addr)
{
    inUn.m_iaddr = inet_addr(addr);
}

const bdUInt bdInAddr::toString(bdNChar8* str, bdUInt size) const
{
    const char* src;

    src = inet_ntoa((in_addr&)*this);
    if (src)
    {
        bdStrlcpy(str, src, size);
        return bdStrlen(str);
    }
    return 0;
}
