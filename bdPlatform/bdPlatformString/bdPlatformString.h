// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

bdUWord bdStrlcpy(bdNChar8* const dst, const bdNChar8* const src, const bdUWord size);
bdUWord bdStrlen(const bdNChar8* const s);
bdUWord bdStrnlen(const bdNChar8* const s, const bdUWord maxLen);
bdNChar8* bdStrchr(bdNChar8* const s, const bdInt c);
const bdNChar8* bdStrchr(const bdNChar8* const s, const bdInt c);
bdNChar8* bdStrstr(bdNChar8* const str, const bdNChar8* const searchStr);
bdInt bdToLower(const bdInt c);
bdNChar8* bdStrlwr(bdNChar8* const s);
int bdSnprintf(char* buf, unsigned int maxlen, const char* format, ...);
bdInt bdVsnprintf(bdNChar8* buf, const bdUWord maxlen, const bdNChar8* format, va_list* argPtr);
int bdPrintf(const char* format, ...);
unsigned int bdStrGetToken(const char* str, const char* delimeters, char* tokenBuffer, unsigned int tokenBufferSize, const char** end);
bool bdDelimSubstr(const char* str, const char* substr, const char* delimeters);
bdInt bdStrncmp(const bdNChar8* const s1, const bdNChar8* s2, const bdInt len);