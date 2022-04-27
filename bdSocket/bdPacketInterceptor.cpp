// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocket.h"

void bdPacketInterceptor::operator delete(void* p)
{
    bdMemory::deallocate(p);
}
