// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdSecurityInfo
{
public:
    static bdUInt bufferToString(const bdUByte8* buffer, const bdUInt bufferLen, bdNChar8* outBuffer, const bdUInt outBufferLen);
    static bdUInt toString(const bdSecurityID& id, bdNChar8* buffer, bdUInt len);
    static bdUInt toString(const bdSecurityKey& key, bdNChar8* buffer, bdUInt len);
};
