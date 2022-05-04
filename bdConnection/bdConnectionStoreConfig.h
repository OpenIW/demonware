// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdConnectionStoreConfig
{
protected:
    bdUInt m_maxReceiveIterations;
    bdFloat32 m_maxShutdownDuration;
    bdUInt m_maxFlushIterations;
public:
    bdConnectionStoreConfig();
    void reset();
};