// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdNATTypeDiscoveryConfig
{
public:
    bdUInt m_NtdcsMaxResends;
    bdFloat32 m_NtdcsSendTimeout;

    bdNATTypeDiscoveryConfig();
    void reset();
    void sanityCheckConfig();
};