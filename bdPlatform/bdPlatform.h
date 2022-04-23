// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#ifdef _WINDOWS
#pragma warning(disable: 4273) // Silence
#define _WINSOCKAPI_    // stops windows.h including winsock.h
#include <Windows.h>
#include <WinSock2.h>
#include <stdio.h>
#endif

#include "bdPlatformCoretypes/bdPlatformCoreTypes.h"
#include "bdPlatformConfig/bdPlatformConfig.h"

#ifdef _WINDOWS
#include "bdPlatformConfig/bdPlatformConfig-win32.h"
#include "bdPlatformConfig/bdPlatformConfig-Win32-MSVC8.h"
#endif

#include "bdPlatformMemory/bdPlatformMemory.h"
#include "bdPlatformPatterns/bdLinkable.h"
#include "bdPlatformError/bdPlatformError.h"
#include "bdPlatformLog/bdPlatformLog.h"
#include "bdPlatformTiming/bdPlatformTiming.h"
#include "bdPlatformThread/bdPlatformThread.h"
#include "bdPlatformThread/bdPlatformMutex.h"
#include "bdPlatformString/bdPlatformString.h"
#include "bdPlatformSocket/bdInAddr.h"
#include "bdPlatformSocket/bdPlatformSocket.h"
#include "bdPlatformSocket/bdPlatformStreamSocket.h"
#include "bdPlatformRandom/bdPlatformTrulyRandom.h"
