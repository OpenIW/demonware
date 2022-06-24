// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdCore/bdCore.h"

bdUInt bdSecurityInfo::bufferToString(const bdUByte8* buffer, const bdUInt bufferLen, bdNChar8* outBuffer, const bdUInt outBufferLen)
{
    bdNChar8* cur;

    cur = outBuffer;
    for (bdUInt i = 0; i < bufferLen; ++i)
    {
        if ((i & 3) == 0)
        {
            if (i)
            {
                cur += bdSnprintf(cur, outBufferLen - (cur - outBuffer), " ");
            }
        }
        if (bdSnprintf(cur, outBufferLen - (cur - outBuffer), "%02hhx") != 2)
        {
            return 0;
        }
        cur += 2;
    }
    return (bdUInt)cur - (bdUInt)outBuffer;
}

bdUInt bdSecurityInfo::toString(const bdSecurityID& id, bdNChar8* buffer, bdUInt len)
{
    return bufferToString(reinterpret_cast<const bdUByte8*>(id.ab), sizeof(bdSecurityID), buffer, len);
}

bdUInt bdSecurityInfo::toString(const bdSecurityKey& key, bdNChar8* buffer, bdUInt len)
{
    return bufferToString(reinterpret_cast<const bdUByte8*>(key.ab), sizeof(bdSecurityKey), buffer, len);
}
