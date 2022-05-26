// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdLobby/bdLobby.h"

bdTaskResult::~bdTaskResult()
{
}

bdBool bdTaskResult::deserialize(bdByteBufferRef)
{
    return false;
}

void bdTaskResult::serialize(bdByteBuffer& buffer)
{
}

bdUInt bdTaskResult::sizeOf()
{
    return 0;
}
