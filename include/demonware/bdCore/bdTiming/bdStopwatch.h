// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdStopwatch
{
protected:
    bdUInt64 m_start;
public:
    bdStopwatch();
    bdFloat32 getElapsedTimeInSeconds();
    void reset();
    void start();
};