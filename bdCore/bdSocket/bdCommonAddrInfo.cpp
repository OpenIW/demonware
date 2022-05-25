// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdCore/bdCore.h"

bdUInt bdCommonAddrInfo::getInfo(bdCommonAddrRef addr, bdNChar8* buf, const bdUInt length)
{
    if (addr.notNull())
    {
        return getInfo(addr.m_ptr, buf, length);
    }
    return 0;
}

bdUInt bdCommonAddrInfo::getInfo(const bdCommonAddr& addr, bdNChar8* buf, const bdUInt length)
{
    bdNChar8* start;
    bdInt written;
    bdNChar8 addrString[22];

    bdNChar8* end = &buf[length];
    switch (addr.getNATType())
    {
    case BD_NAT_STRICT:
        written = bdSnprintf(buf, end - buf, "NAT Type: %s\n", "BD_NAT_STRICT");
        break;
    case BD_NAT_MODERATE:
        written = bdSnprintf(buf, (end - buf), "NAT Type: %s\n", "BD_NAT_MODERATE");
        break;
    case BD_NAT_OPEN:
        written = bdSnprintf(buf, (end - buf), "NAT Type: %s\n", "BD_NAT_OPEN");
        break;
    default:
        written = bdSnprintf(buf, (end - buf), "NAT Type: %s\n", "**UNKNOWN**");
        break;
    }
    if (written <= 0 || written > end - buf)
    {
        buf = end - 1;
        if (length)
        {
            *buf = 0;
        }
    }
    else
    {
        buf += written;
    }
    written = bdSnprintf(buf, (end - buf), "%u Local Addrs Found: \n", addr.getLocalAddrs().getSize());
    if (written <= 0 || written > end - buf)
    {
        buf = end - 1;
        if (length)
        {
            *buf = 0;
        }
    }
    else
    {
        buf += written;
    }

    for (bdUInt i = 0; i < addr.getLocalAddrs().getSize(); ++i)
    {
        addr.getLocalAddrByIndex(i).toString(addrString, sizeof(addrString));
        written = bdSnprintf(buf, (end - buf), "Local Addr %u :  %s \n", i, addrString);
        if (written <= 0 || written > end - buf)
        {
            buf = end - 1;
            if (length)
            {
                *buf = 0;
            }
        }
        else
        {
            buf += written;
        }
    }
    addr.getPublicAddr().toString(addrString, sizeof(addrString));
    written = bdSnprintf(buf, (end - buf), "Public Addr :  %s \n", addrString);
    if (written <= 0 || written > end - buf)
    {
        buf = end - 1;
        if (length)
        {
            *buf = 0;
        }
    }
    else
    {
        buf += written;
    }

    written = bdSnprintf(buf, (end - buf), "Addr is loopback : %s  -- Addr Hash : %u \n", addr.isLoopback() ? "TRUE" : "FALSE", addr.getHash());
    if (written <= 0 || written > end - buf)
    {
        buf = end - 1;
        if (length)
        {
            *buf = 0;
        }
    }
    else
    {
        buf += written;
    }
    return length - (end - buf);
}

bdUInt bdCommonAddrInfo::getBriefInfo(const bdCommonAddrRef addr, bdNChar8* buf, const bdUInt length)
{
    if (addr.notNull())
    {
        return getBriefInfo(*addr, buf, length);
    }
    return 0;
}

bdUInt bdCommonAddrInfo::getBriefInfo(const bdCommonAddr& addr, bdNChar8* buf, const bdUInt length)
{
    bdNChar8* start;
    bdInt written;
    bdNChar8 addrsString[120];
    bdNChar8 addrString[22];

    bdNChar8* end = &buf[length];
    if (addr.getPublicAddr().getAddress().isValid())
    {
        addr.getPublicAddr().toString(addrString, sizeof(addrString));
        written = bdSnprintf(buf, length, "Public Addr: %s\n", addrString);
    }
    else
    {
        if (addr.getLocalAddrs().getSize())
        {
            start = addrsString;
            bdAssert(addr.getLocalAddrs().getSize() <= BD_MAX_LOCAL_ADDRS, "bdCommonAddr invalid.");
            for (bdUInt i = 0; i < addr.getLocalAddrs().getSize(); ++i)
            {
                start += addr.getLocalAddrByIndex(i).toString(start, 120 - (start - addrsString));
                start += bdSnprintf(start, 120 - (start - addrsString), ", ");
            }
            *(start - 2) = 0;
            written = bdSnprintf(buf, length, "Local Addr: %s\n", addrsString);
        }
        else
        {
            written = bdSnprintf(buf, length, "Empty Common Addr\n");
        }
    }
    if (written <= 0 || written > end - buf)
    {
        buf = end - 1;
        if (length)
        {
            *buf = 0;
        }
    }
    else
    {
        buf = written + buf;
    }
    return length - (end - buf);
}
