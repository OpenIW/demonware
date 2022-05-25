// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdPlatform/bdPlatform.h"

bdCustomAssertHandler g_CustomAssertHandler;

void bdRegisterCustomAssertHandler(const bdCustomAssertHandler customFunction)
{
    g_CustomAssertHandler = customFunction;
}

void bdHandleAssert(const bdBool expr, const bdNChar8* const exprString, const bdNChar8* const file, const bdNChar8* const function, const bdUInt line, const bdNChar8* const format, ...)
{
    va_list args;
    bdNChar8 message[256];

    if (g_CustomAssertHandler)
    {
        va_start(args, format);
        g_CustomAssertHandler(expr, exprString, file, function, line, format, args);
    }
    else if (!expr)
    {
        va_start(args, format);
        bdMemset(message, 0, sizeof(message));
        if (bdVsnprintf(message, sizeof(message), format, &args) >= sizeof(message))
        {
            bdLogWarn("bdPlatformError", "Assert Message truncated");
        }
        if (exprString)
        {
            bdLogError(NULL, "BD_ASSERT(%s) failed. \"%s\"", exprString, message);
        }
        else
        {
            bdLogError(NULL, "BD_ASSERT failed. \"%s\"", message);
        }
    }
}
