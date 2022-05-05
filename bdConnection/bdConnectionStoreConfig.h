// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdConnectionStoreConfig
{
public:
    bdUInt m_maxReceiveIterations;
    bdFloat32 m_maxShutdownDuration;
    bdUInt m_maxFlushIterations;

    bdConnectionStoreConfig();
    void reset();
};