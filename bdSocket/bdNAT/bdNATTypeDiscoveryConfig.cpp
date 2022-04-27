// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdSocket/bdSocket.h"

bdNATTypeDiscoveryConfig::bdNATTypeDiscoveryConfig()
{
    m_NtdcsMaxResends = 5;
    m_NtdcsSendTimeout = 0.5f;
}

void bdNATTypeDiscoveryConfig::reset()
{
    m_NtdcsMaxResends = 5;
    m_NtdcsSendTimeout = 0.5f;
}

void bdNATTypeDiscoveryConfig::sanityCheckConfig()
{
    if (m_NtdcsMaxResends > 10 || m_NtdcsMaxResends < 2)
    {
        bdLogWarn("NATTypeDiscoveryConfig", "Max resends configured to %u. This is outside the normal recommended range", m_NtdcsMaxResends);
    }
    if (m_NtdcsSendTimeout > 2.0 || m_NtdcsSendTimeout < 0.05000000074505806)
    {
        bdLogWarn("NATTypeDiscoveryConfig", "Send timeout configured to %f. This is outside the normal recommended range", m_NtdcsSendTimeout);
    }
}
