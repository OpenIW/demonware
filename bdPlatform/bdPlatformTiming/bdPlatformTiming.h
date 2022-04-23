// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdPlatformTiming
{
public:
    static LARGE_INTEGER getHiResTimeStamp();
    static float getElapsedTime(bdUInt64 time1, bdUInt64 time2);
    static void sleep(unsigned int timeInMs);
    static unsigned int getLoResElapsedTime(unsigned int time1, unsigned int time2);
};