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
bdInt bdSnprintf(bdNChar8* buf,bdUInt maxlen, const bdNChar8* format, ...);
bdInt bdVsnprintf(bdNChar8* buf, const bdUWord maxlen, const bdNChar8* format, va_list* argPtr);
bdInt bdPrintf(const bdNChar8* format, ...);
bdUInt bdStrGetToken(const bdNChar8* str, const bdNChar8* delimeters, bdNChar8* tokenBuffer, bdUInt tokenBufferSize, const bdNChar8** end);
bdBool bdDelimSubstr(const bdNChar8* str, const bdNChar8* substr, const bdNChar8* delimeters);
bdInt bdStrncmp(const bdNChar8* const s1, const bdNChar8* s2, const bdInt len);
