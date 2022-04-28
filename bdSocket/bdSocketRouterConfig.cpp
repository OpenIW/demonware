// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocket.h"

bdSocketRouterConfig::bdSocketRouterConfig() : m_ignoreConnectionReset(true), m_DTLSAssociationReceiveTimeout(121.0f), m_maxConnectionResets(10)
{
}

void bdSocketRouterConfig::reset()
{
    m_ignoreConnectionReset = 1;
    m_DTLSAssociationReceiveTimeout = 121.0;
    m_maxConnectionResets = 10;
}

void bdSocketRouterConfig::sanityCheckConfig()
{
    if (!m_ignoreConnectionReset)
    {
        if (m_ignoreConnectionReset)
        {
            bdLogWarn("socketRouterConfig", "Ignore connection reset differs from recommended setting of %s", "False");
        }
        else
        {
            bdLogWarn("socketRouterConfig", "Ignore connection reset differs from recommended setting of %s", "True");
        }
    }
    if (m_DTLSAssociationReceiveTimeout > 150.0 || m_DTLSAssociationReceiveTimeout < 10.0)
    {
        bdLogWarn("socketRouterConfig", "Receive timeout configured to %f. This is outside the normal recommended range", m_DTLSAssociationReceiveTimeout);
    }
    if (m_maxConnectionResets > 100 || !m_maxConnectionResets)
    {
        bdLogWarn("socketRouterConfig", "Max connection resets to ignore configured to %u. This is outside the normal recommended range", m_maxConnectionResets);
    }
}
