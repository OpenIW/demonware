// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdPlatform/bdPlatform.h"

LARGE_INTEGER bdPlatformTiming::getHiResTimeStamp()
{
    LARGE_INTEGER time;

    QueryPerformanceCounter(&time);
    return time;
}

float bdPlatformTiming::getElapsedTime(bdUInt64 time1, bdUInt64 time2)
{
    LARGE_INTEGER frequency;

    QueryPerformanceFrequency(&frequency);
    return (time2 - time1) / frequency.QuadPart;
}

void bdPlatformTiming::sleep(unsigned int timeInMs)
{
    Sleep(timeInMs);
}

unsigned int bdPlatformTiming::getLoResElapsedTime(unsigned int time1, unsigned int time2)
{
    if ( time2 < time1 )
    {
        time2 += 0xFFFFFFFF;
    }
    return (time2 - time1) / 1000;
}
