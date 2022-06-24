// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdPlatformTiming
{
public:
    static LARGE_INTEGER getHiResTimeStamp();
    static bdFloat32 getElapsedTime(bdUInt64 time1, bdUInt64 time2);
    static void sleep(bdUInt timeInMs);
    static bdUInt getLoResElapsedTime(bdUInt time1, bdUInt time2);
};
