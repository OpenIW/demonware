// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdNATTravListener
{
public:
    virtual ~bdNATTravListener() {};
    virtual void onNATAddrDiscovery(bdReference<bdCommonAddr>, const bdAddr&) {};
    virtual void onNATAddrDiscoveryFailed(bdReference<bdCommonAddr>) {};
};
