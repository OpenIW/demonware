// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdNet/bdNet.h"

void bdLANDiscoveryListener::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

bdLANDiscoveryListener::~bdLANDiscoveryListener()
{
}

void bdLANDiscoveryListener::onDiscovery(bdReference<bdGameInfo> __formal)
{
    __formal.~bdReference();
}

void bdLANDiscoveryListener::onDiscoveryFinished()
{
    ;
}

void bdLANDiscoveryListener::onRequest()
{
    ;
}
