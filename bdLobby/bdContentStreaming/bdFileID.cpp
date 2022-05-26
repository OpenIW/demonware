// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

bdFileID::bdFileID()
    : bdTaskResult(), m_fileID(0)
{
}

bdFileID::~bdFileID()
{
}

void bdFileID::serialize(bdByteBuffer& buffer)
{
    buffer.writeUInt64(m_fileID);
}

bdBool bdFileID::deserialize(bdByteBufferRef buffer)
{
    return buffer->readUInt64(m_fileID);
}

bdUInt bdFileID::sizeOf()
{
    return sizeof(bdFileID);
}
