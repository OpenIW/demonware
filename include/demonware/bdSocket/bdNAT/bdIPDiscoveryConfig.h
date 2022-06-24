// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdIPDiscoveryConfig
{
public:
    bdUByte8 m_retries;
    bdFloat32 m_requestTimeout;

    bdIPDiscoveryConfig();
    void sanityCheckConfig();
    void reset();
};