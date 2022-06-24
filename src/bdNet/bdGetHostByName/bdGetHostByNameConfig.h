// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdGetHostByNameConfig
{
public:
    bdFloat32 m_timeout;
    bdGetHostByNameConfig();
    bdGetHostByNameConfig(bdFloat32 timeout);
    void sanityCheckConfig();
    void reset();
};