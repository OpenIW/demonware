// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

bdBool bdGroupCount::deserialize(bdByteBufferRef buffer)
{
    return buffer->readUInt32(m_groupID) && buffer->readUInt32(m_groupCount);
}

void bdGroupCount::serialize(bdByteBuffer& buffer)
{
    buffer.writeUInt32(m_groupID);
    buffer.writeUInt32(m_groupCount);
}

bdUInt bdGroupCount::sizeOf()
{
    return sizeof(bdGroupCount);
}
