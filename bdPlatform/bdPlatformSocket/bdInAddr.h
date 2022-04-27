// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdInAddr
{
public:
    bdInAddr();
    bdInAddr(const bdNChar8* addr);
    void fromString(const char* addr);
    const bdUInt toString(char* str, unsigned int size) const;
    union
    {
        struct
        {
            char m_b1;
            char m_b2;
            char m_b3;
            char m_b4;
        } m_caddr;
        unsigned int m_iaddr;
    } inUn;
};