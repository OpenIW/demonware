// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdQoSProbeListener
{
public:
    virtual ~bdQoSProbeListener();
    virtual void onQoSProbeSuccess(bdQoSProbeInfo& info);
    virtual void onQoSProbeFailed(bdCommonAddrRef addr);
    void operator delete(void* p);
};
