// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdPlatform/bdPlatform.h"

unsigned int bdStrlcpy(char* dst, const char* src, size_t size)
{
    unsigned int slen;

    slen = strlen(src);
    if (size && dst)
    {
        if (slen >= size - 1)
        {
            slen = size - 1;
        }
        memcpy(dst, (char*)src, slen);
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

int bdSnprintf(char* buf, unsigned int maxlen, const char* format, ...)
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

int bdPrintf(const char* format, ...)
{
    va_list va;

    va_start(va, format);
    return vfprintf(stdout + 1, format, va);
}

unsigned int bdStrGetToken(const char* str, const char* delimeters, char* tokenBuffer, unsigned int tokenBufferSize, const char** end)
{
    int tokenSize;
    int offset = 0;

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

bool bdDelimSubstr(const char* str, const char* substr, const char* delimeters)
{
    bool isSubStr = true;
    char subStrToken[68];
    char strToken[68];
    unsigned int token;

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
