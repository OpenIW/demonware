// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdSocket/bdSocketRouter.h"

bdIPDiscoveryConfig::bdIPDiscoveryConfig()
{
    m_retries = 5;
    m_requestTimeout = 0.5f;
}

void bdIPDiscoveryConfig::sanityCheckConfig()
{
    if (m_retries > 10 || m_retries < 2u)
    {
        bdLogWarn("IPDiscoveryConfig", "Max resends configured to %u. This is outside the normal recommended range", m_retries);
    }
    if (m_requestTimeout > 2.0 || m_requestTimeout < 0.05000000074505806)
    {
        bdLogWarn("IPDiscoveryConfig", "Send timeout configured to %f. This is outside the normal recommended range", m_requestTimeout);
    }
}

void bdIPDiscoveryConfig::reset()
{
    m_retries = 5;
    m_requestTimeout = 0.5f;
}
