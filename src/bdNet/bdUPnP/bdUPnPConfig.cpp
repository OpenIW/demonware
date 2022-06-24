// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdNet/bdNet.h"

bdUPnPConfig::bdUPnPConfig()
    : m_discoveryTimeout(3.0f), m_responseTimeout(10.0f), m_connectTimeout(5.0f), m_runMode(BD_UPNP_DO_PORT_MAPPING), m_discoveryRetries(1), m_disabled(false), m_gatewayAddr()
{
}

bdUPnPConfig::bdUPnPConfig(const bdUPnPConfig* other)
    : m_discoveryTimeout(other->m_discoveryTimeout), m_responseTimeout(other->m_responseTimeout), m_connectTimeout(other->m_connectTimeout), m_runMode(other->m_runMode), 
    m_discoveryRetries(other->m_discoveryRetries), m_disabled(other->m_disabled), m_gatewayAddr(other->m_gatewayAddr)
{
}

bdUPnPConfig::~bdUPnPConfig()
{
}

void bdUPnPConfig::reset()
{
    m_discoveryTimeout = 3.0;
    m_responseTimeout = 10.0;
    m_connectTimeout = 5.0;
    m_runMode = BD_UPNP_DO_PORT_MAPPING;
    m_discoveryRetries = 1;
    m_gatewayAddr = bdInetAddr();
}

void bdUPnPConfig::sanityCheckConfig()
{
    if (m_discoveryTimeout > 12.0 || m_discoveryTimeout < 0.5)
        bdLogWarn("UPnPConfig", "UPnP discovery timeout set to %f seconds. This is outside the normal recommended range", m_discoveryTimeout);
    if (m_responseTimeout > 40.0 || m_responseTimeout < 1.0)
        bdLogWarn("UPnPConfig", "UPnP response timeout set to % f seconds.This is outside the normal recommended range", m_responseTimeout);
    if (m_connectTimeout > 20.0 || m_connectTimeout < 1.0)
        bdLogWarn("UPnPConfig", "UPnP connect timeout set to %f seconds. This is outside the normal recommended range", m_connectTimeout);
    if (m_runMode && m_runMode != BD_UPNP_EXTERNAL_IP_ONLY)
        bdLogWarn("UPnPConfig", "UPnP run mode misconfigured. This will cause undefined behaviour");
    if (m_discoveryRetries > 4)
        bdLogWarn("UPnPConfig", "UPnP discovery retries set to %u . This is outside the normal recommended range", m_discoveryRetries);
}
