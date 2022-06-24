// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

const bdNChar8* skipWhitespace(const bdNChar8* ptr);
bdUInt findIndex(bdNChar8 c, const bdNChar8* cvt);

class bdBase64
{
public:
    static void encode(const bdByte8* src, const bdUInt srcLen, bdNChar8* dest, const bdUInt destLen);
    static void encode(const bdByte8* src, const bdUInt srcLen, bdNChar8* dest, const bdUInt destLen, const bdNChar8* alphabet);
    static void encodeURLSafe(const bdByte8* src, const bdUInt srcLen, bdNChar8* dest, const bdUInt destLen);
    static bdUInt decode(const bdNChar8* src, const bdUInt srcLen, bdByte8* dest, const bdUInt destLen);
    static bdUInt decode(const bdNChar8* src, const bdUInt srcLen, bdByte8* dest, const bdUInt destLen, const bdNChar8* alphabet);
    static bdUInt decodeURLSafe(const bdNChar8* src, const bdUInt srcLen, bdByte8* dest, const bdUInt destLen);
};