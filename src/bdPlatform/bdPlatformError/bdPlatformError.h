// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

typedef void (*bdCustomAssertHandler)(const bdBool, const bdNChar8* const, const bdNChar8* const, const bdNChar8* const, const bdUInt, const bdNChar8* const, va_list);

void bdRegisterCustomAssertHandler(const bdCustomAssertHandler customFunction);
void bdHandleAssert(const bdBool expr, const bdNChar8* const exprString, const bdNChar8* const file, const bdNChar8* const function, const bdUInt line, const bdNChar8* const format, ...);

#define bdAssert(cond, Msg, ...) bdHandleAssert(cond, #cond, __FILE__, __FUNCTION__, __LINE__, Msg, ##__VA_ARGS__)