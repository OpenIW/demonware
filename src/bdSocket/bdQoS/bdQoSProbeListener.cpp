// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocket.h"

bdQoSProbeListener::~bdQoSProbeListener()
{
}

void bdQoSProbeListener::onQoSProbeSuccess(bdQoSProbeInfo& info)
{
}

void bdQoSProbeListener::onQoSProbeFailed(bdCommonAddrRef addr)
{
}

void bdQoSProbeListener::operator delete(void* p)
{
    bdMemory::deallocate(p);
}
