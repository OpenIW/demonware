// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdCommonAddrInfo
{
public:
    static bdUInt getInfo(const bdCommonAddrRef addr, bdNChar8* buf, const bdUInt length);
    static bdUInt getInfo(const bdCommonAddr& addr, bdNChar8* buf, const bdUInt length);
    static bdUInt getBriefInfo(const bdCommonAddrRef addr, bdNChar8* buf, const bdUInt length);
    static bdUInt getBriefInfo(const bdCommonAddr& addr, bdNChar8* buf, const bdUInt length);
};
