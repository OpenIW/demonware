// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdCore/bdCore.h"

const bdNChar8* standardBase64Alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const bdNChar8* urlsafeBase64Alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

const bdNChar8* skipWhitespace(const bdNChar8* ptr)
{
    bdBool whitespace;

    if (ptr != NULL)
    {
        while (true)
        {
            if ((*ptr < '\t') || (whitespace = true, '\r' < *ptr)) {
                whitespace = *ptr == ' ';
            }
            if (!whitespace) break;
            ptr = ptr + 1;
        }
    }
    return ptr;
}

bdUInt findIndex(bdNChar8 c, const bdNChar8* cvt)
{
    if (c >= 'A' && c <= 'Z')
    {
        return c - 65;
    }
    if (c >= 'a' && c <= 'z')
    {
        return c - 97 + 26;
    }
    if (c >= '0' && c <= '9')
    {
        return c - 48 + 52;
    }
    if (c == cvt[62])
    {
        return 62;
    }
    if (c == cvt[63])
    {
        return 63;
    }
    bdLogWarn("bdCore/bdUtilities/bdBase64", "Unknown base64 character ['%c'] - treating as 'A'", c);
    return 0;
}

void bdBase64::encode(const bdByte8* src, const bdUInt srcLen, bdNChar8* dest, const bdUInt destLen)
{
    encode(src, srcLen, dest, destLen, standardBase64Alphabet);
}

void bdBase64::encode(const bdByte8* src, const bdUInt srcLen, bdNChar8* dest, const bdUInt destLen, const bdNChar8* alphabet)
{
    bdNChar8 c;
    bdUInt i;
    bdUInt destIndex;

    //bdAssert(src != BD_NULL, "bdBase64::encode() src may not be NULL");
    //bdAssert(dest != BD_NULL, "bdBase64::encode() dest may not be NULL");
    //bdAssert(destLen >= BD_BIN_TO_B64_SIZE(srcLen), "bdBase64:encode() Dest buffer too small: [%d]bytes vs [%d] minimum", destLen, BD_BIN_TO_B64_SIZE(srcLen));
    //bdAssert(alphabet != BD_NULL, "bdBase64::encode() alphabet may not be NULL");

    destIndex = 0;
    for (i = 0; i < srcLen; ++i)
    {
        if (destIndex >= destLen)
        {
            break;
        }
        dest[destIndex++] = alphabet[(src[i] >> 2) & 0x3F];
        c = (16 * src[i++]) & 0x3F;
        if (i < srcLen)
        {
            c |= (src[i] >> 4) & 0xF;
        }
        if (destIndex >= destLen)
        {
            break;
        }
        dest[destIndex++] = alphabet[c];
        if (i >= srcLen)
        {
            ++i;
            if (destIndex >= destLen)
            {
                break;
            }
            dest[destIndex++] = 61;
        }
        else
        {
            c = (4 * src[i++]) & 0x3F;
            if (i < srcLen)
            {
                c |= (src[i] >> 6) & 3;
            }
            if (destIndex >= destLen)
            {
                break;
            }
            dest[destIndex++] = alphabet[c];
        }
        if (i >= srcLen)
        {
            if (destIndex >= destLen)
            {
                break;
            }
            dest[destIndex++] = 61;
        }
        else
        {
            if (destIndex >= destLen)
            {
                break;
            }
            dest[destIndex++] = alphabet[src[i] & 0x3F];
        }
    }
    if (destIndex >= destLen)
    {
        destIndex = destLen - 1;
        //bdLog(BD_LOG_WARNING, "bdBase64::encode() Truncated: Dest buffer too small: [%d]bytes vs [%d] minimum", destLen, BD_BIN_TO_B64_SIZE(srcLen));
    }
    dest[destIndex] = 0;
}

void bdBase64::encodeURLSafe(const bdByte8* src, const bdUInt srcLen, bdNChar8* dest, const bdUInt destLen)
{
    encode(src, srcLen, dest, destLen, urlsafeBase64Alphabet);
}

bdUInt bdBase64::decode(const bdNChar8* src, const bdUInt srcLen, bdByte8* dest, const bdUInt destLen)
{
    return decode(src, srcLen, dest, destLen, standardBase64Alphabet);
}

bdUInt bdBase64::decode(const bdNChar8* src, const bdUInt srcLen, bdByte8* dest, const bdUInt destLen, const bdNChar8* alphabet)
{
    bdNChar8 c1;
    bdNChar8 c;
    const bdNChar8* ptr;
    bdUInt destIndex;
    const bdNChar8* endPtr;
    bdUInt expectedLen;

    //bdAssert(src != BD_NULL, "bdBase64::decode() src may not be NULL");
    //bdAssert(dest != BD_NULL, "bdBase64::decode() dest may not be NULL");
    //bdAssert(alphabet != BD_NULL, "bdBase64::decode() alphabet may not be NULL");

    expectedLen = (3 * srcLen) >> 2;
    if (srcLen && src)
    {
        for (endPtr = &src[srcLen - 1]; *endPtr == 61; --endPtr)
        {
            --expectedLen;
        }
        //bdAssert(destLen >= expectedLen, "bdBase64::decode() Dest buffer too small: [%d]bytes vs [%d] minimum", destLen, expectedLen);
    }
    destIndex = 0;
    for (ptr = skipWhitespace(src); ptr != &src[srcLen]; ptr = skipWhitespace(ptr + 1))
    {
        if (*ptr == 61)
        {
            break;
        }
        c = findIndex(*ptr, alphabet);
        ptr = skipWhitespace(ptr + 1);
        c1 = findIndex(*ptr, alphabet);
        c = (c1 >> 4) & 3 | (4 * c);
        if (destIndex >= destLen)
        {
            break;
        }
        dest[destIndex++] = c;
        ptr = skipWhitespace(ptr + 1);
        if ((int)ptr - (int)src < srcLen)
        {
            if (*ptr == 61)
            {
                break;
            }
            c = findIndex(*ptr, alphabet);
            c1 = (c >> 2) & 0xF | (16 * c1);
            if (destIndex >= destLen)
            {
                break;
            }
            dest[destIndex++] = c1;
        }
        ptr = skipWhitespace(ptr + 1);
        if ((int)ptr - (int)src < srcLen)
        {
            if (*ptr == 61)
            {
                break;
            }
            c = findIndex(*ptr, alphabet) | (c << 6);
            if (destIndex >= destLen)
            {
                break;
            }
            dest[destIndex++] = c;
        }
    }
    if (destIndex > destLen)
    {
        //bdLogWarning("bdBase64::decode() Truncated: Dest buffer too small: [%d]bytes vs [%d] minimum", destLen, destIndex);
    }
    return destIndex;
}

bdUInt bdBase64::decodeURLSafe(const bdNChar8* src, const bdUInt srcLen, bdByte8* dest, const bdUInt destLen)
{
    return decode(src, srcLen, dest, destLen, urlsafeBase64Alphabet);
}