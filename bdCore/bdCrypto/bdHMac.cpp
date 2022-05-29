// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdCore/bdCore.h"

bdHMac::bdHMac()
{
}

bdHMac::~bdHMac()
{
}

void bdHMac::operator delete(void* p)
{
    bdMemory::deallocate(p);
}
