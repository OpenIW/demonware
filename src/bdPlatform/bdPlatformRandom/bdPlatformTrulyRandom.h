// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdTrulyRandomImpl
{
public:
    bdTrulyRandomImpl();
    ~bdTrulyRandomImpl();
    bdUWord getRandomUInt();
    void getRandomUByte8(bdUByte8* pbBuffer, bdUWord dwLen);
};

void bdGetRandomUChar8(bdUByte8* in, bdUInt32 length);
