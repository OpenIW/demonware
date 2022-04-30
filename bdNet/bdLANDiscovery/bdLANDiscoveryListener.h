// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdLANDiscoveryListener
{
public:
    void operator delete(void* p);
    virtual ~bdLANDiscoveryListener();
    virtual void onDiscovery(bdReference<bdGameInfo> __formal);
    virtual void onDiscoveryFinished();
    virtual void onRequest();
};