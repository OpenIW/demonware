// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdSocketRouterConfig
{
public:
    bdBool m_ignoreConnectionReset;
    bdFloat32 m_DTLSAssociationReceiveTimeout;
    bdUInt m_maxConnectionResets;

    bdSocketRouterConfig();
    void reset();
    void sanityCheckConfig();
};
