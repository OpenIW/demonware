// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdPlatform/bdPlatform.h"

HCRYPTPROV phProv;

bdTrulyRandomImpl::bdTrulyRandomImpl()
{
    CryptAcquireContext(&phProv, 0, 0, 1, 0xF0000000);
}

bdTrulyRandomImpl::~bdTrulyRandomImpl()
{
    CryptReleaseContext(phProv, 0);
}

bdUWord bdTrulyRandomImpl::getRandomUInt()
{
    bdUWord pbBuffer;

    getRandomUByte8(reinterpret_cast<bdUByte8*>(&pbBuffer), sizeof(bdUWord));
    return pbBuffer;
}

void bdTrulyRandomImpl::getRandomUByte8(bdUByte8* pbBuffer, bdUWord dwLen)
{
    bdGetRandomUChar8(pbBuffer, dwLen);
}

bdTrulyRandomImpl s_randomInit = bdTrulyRandomImpl();

void bdGetRandomUChar8(bdUByte8* in, bdUInt32 length)
{
    CryptGenRandom(phProv, length, in);
}
