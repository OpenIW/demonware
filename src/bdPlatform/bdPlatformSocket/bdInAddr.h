// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdInAddr
{
public:
    bdInAddr();
    bdInAddr(const bdNChar8* addr);
    void fromString(const bdNChar8* addr);
    const bdUInt toString(bdNChar8* str, bdUInt size) const;
    union
    {
        struct
        {
            bdUByte8 m_b1;
            bdUByte8 m_b2;
            bdUByte8 m_b3;
            bdUByte8 m_b4;
        } m_caddr;
        bdUInt m_iaddr;
    } inUn;
};
