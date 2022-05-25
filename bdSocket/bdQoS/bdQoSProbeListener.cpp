// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocket.h"

bdQoSProbeListener::~bdQoSProbeListener()
{
}

void bdQoSProbeListener::onQoSProbeSuccess(const bdQoSProbeInfo&)
{
}

void bdQoSProbeListener::onQoSProbeFailed(bdCommonAddrRef)
{
}

void bdQoSProbeListener::operator delete(void* p)
{
    bdMemory::deallocate(p);
}
