// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdNet/bdNet.h"

bdGetHostByNameConfig::bdGetHostByNameConfig()
{
    m_timeout = 10.0f;
}

void bdGetHostByNameConfig::sanityCheckConfig()
{
    if (m_timeout > 40.0 || m_timeout < 1.0)
    {
        bdLogWarn("getHostByNameConfig", "Lookup timeout set to %f seconds. This is outside the normal recommended range", m_timeout);
    }
}

void bdGetHostByNameConfig::reset()
{
    m_timeout = 10.0f;
}
