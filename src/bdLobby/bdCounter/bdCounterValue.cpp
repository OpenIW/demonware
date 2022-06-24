// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

bdCounterValue::bdCounterValue()
    : bdTaskResult(), m_counterID(0), m_counterValue(0)
{
}

bdUInt bdCounterValue::sizeOf()
{
    return sizeof(bdCounterValue);
}

void bdCounterValue::serialize(bdByteBuffer& buffer)
{
    buffer.writeUInt32(m_counterID);
    buffer.writeInt64(m_counterValue);
}

bdBool bdCounterValue::deserialize(bdByteBufferRef buffer)
{
    return buffer->readUInt32(m_counterID) && buffer->readInt64(m_counterValue);
}
