// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdCore/bdCore.h"

bdStopwatch::bdStopwatch()
{
    reset();
}

bdFloat32 bdStopwatch::getElapsedTimeInSeconds()
{
    if (!m_start)
    {
        return 0.0f;
    }
    LARGE_INTEGER nowTimeStamp = bdPlatformTiming::getHiResTimeStamp();
    return bdPlatformTiming::getElapsedTime(m_start, nowTimeStamp.QuadPart);
}

void bdStopwatch::reset()
{
    m_start = 0;
}

void bdStopwatch::start()
{
    m_start = bdPlatformTiming::getHiResTimeStamp().QuadPart;
}
