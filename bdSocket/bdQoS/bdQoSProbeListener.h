// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdQoSProbeListener
{
public:
    virtual ~bdQoSProbeListener();
    virtual void onQoSProbeSuccess(const bdQoSProbeInfo*);
    virtual void onQoSProbeFailed(bdCommonAddrRef);
    void operator delete(void* p);
};