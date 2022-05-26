// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdPlatform/bdPlatform.h"

bdUWord bdStrlcpy(bdNChar8* const dst, const bdNChar8* const src, const bdUWord size)
{
    bdUInt slen;

    slen = bdStrlen(src);
    if (size && dst)
    {
        if (slen >= size - 1)
        {
            slen = size - 1;
        }
        bdMemcpy(dst, src, slen);
        dst[slen] = 0;
    }
    return slen;
}

bdUWord bdStrlen(const bdNChar8* const s)
{
    return strlen(s);
}

bdUWord bdStrnlen(const bdNChar8* const s, const bdUWord maxLen)
{
    const bdNChar8* pos;

    pos = reinterpret_cast<const bdNChar8*>(memchr(s, 0, maxLen));
    return pos ? pos - s : maxLen;
}

bdNChar8* bdStrchr(bdNChar8* const s, const bdInt c)
{
    return strchr(s, c);
}

const bdNChar8* bdStrchr(const bdNChar8* const s, const bdInt c)
{
    return strchr(s, c);
}

bdNChar8* bdStrstr(bdNChar8* const str, const bdNChar8* const searchStr)
{
    return strstr(str, searchStr);
}

bdInt bdToLower(const bdInt c)
{
    return tolower(c);
}

bdNChar8* bdStrlwr(bdNChar8* const s)
{
    for (bdUInt i = 0; s[i]; ++i)
    {
        s[i] = bdToLower(s[i]);
    }
    return s;
}

bdInt bdSnprintf(bdNChar8* buf, bdUInt maxlen, const bdNChar8* format, ...)
{
    bdInt ret;
    va_list va;

    va_start(va, format);
    ret = _vscprintf(format, va);
    vsnprintf_s(buf, maxlen, 0xFFFFFFFF, format, va);
    return ret;
}

bdInt bdVsnprintf(bdNChar8* buf, const bdUWord maxlen, const bdNChar8* format, va_list* argPtr)
{
    return vsnprintf(buf, maxlen, format, *argPtr);
}

bdInt bdPrintf(const bdNChar8* format, ...)
{
    static char s_buf[1024];
    va_list va;

    va_start(va, format);
    _vsnprintf(s_buf, sizeof(s_buf), format, va);
    va_end(va);
    OutputDebugString(s_buf);
    return 0;
}

bdUInt bdStrGetToken(const bdNChar8* str, const bdNChar8* delimeters, bdNChar8* tokenBuffer, bdUInt tokenBufferSize, const bdNChar8** end)
{
    bdInt tokenSize;
    bdInt offset = 0;

    if (str)
    {
        tokenSize = strspn(str, delimeters);
        offset = strcspn(&str[tokenSize], delimeters);
        if (offset + 1 >= tokenBufferSize)
        {
            bdStrlcpy(tokenBuffer, &str[tokenSize], tokenBufferSize);
        }
        else
        {
            bdStrlcpy(tokenBuffer, &str[tokenSize], offset + 1);
        }

        if (end)
        {
            *end = &str[tokenSize + offset];
        }
    }
    return offset;
}

bdBool bdDelimSubstr(const bdNChar8* str, const bdNChar8* substr, const bdNChar8* delimeters)
{
    bdBool isSubStr = true;
    bdNChar8 subStrToken[68];
    bdNChar8 strToken[68];
    bdUInt token;

    while (isSubStr)
    {
        memset(subStrToken, 0, 64);
        token = bdStrGetToken(substr, delimeters, subStrToken, 64, &substr);
        if (token < 64 && token)
        {
            memset(strToken, 0, 64);
            if (bdStrGetToken(str, delimeters, strToken, 64, &str) < 64)
            {
                isSubStr = !_stricmp(subStrToken, strToken);
            }
        }
        else
        {
            break;
        }
    }
    return isSubStr;
}

bdInt bdStrncmp(const bdNChar8* const s1, const bdNChar8* s2, const bdInt len)
{
    return strncmp(s1, s2, len);
}
