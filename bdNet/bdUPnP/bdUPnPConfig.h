// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdUPnPConfig
{
public:
    enum bdUPnPRunMode : bdInt
    {
        BD_UPNP_DO_PORT_MAPPING = 0x0,
        BD_UPNP_EXTERNAL_IP_ONLY = 0x1,
    };
    bdFloat32 m_discoveryTimeout;
    bdFloat32 m_responseTimeout;
    bdFloat32 m_connectTimeout;
    bdUPnPRunMode m_runMode;
    bdUInt m_discoveryRetries;
    bdBool m_disabled;
    bdInetAddr m_gatewayAddr;

    bdUPnPConfig();
    bdUPnPConfig(const bdUPnPConfig* other);
    ~bdUPnPConfig();
    void reset();
    void sanityCheckConfig();
};