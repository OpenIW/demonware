// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdLobby/bdLobby.h"

bdTaskResult::~bdTaskResult()
{
}

bdBool bdTaskResult::deserialize(bdByteBufferRef)
{
    return false;
}

bdUInt bdTaskResult::sizeOf()
{
    return 0;
}
