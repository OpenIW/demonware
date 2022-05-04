// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdConnection/bdConnection.h"

bdConnectionStoreConfig::bdConnectionStoreConfig() : m_maxReceiveIterations(100), m_maxShutdownDuration(5.0f), m_maxFlushIterations(5)
{
}

void bdConnectionStoreConfig::reset()
{
    m_maxReceiveIterations = 100;
    m_maxShutdownDuration = 5.0;
    m_maxFlushIterations = 5;
}
