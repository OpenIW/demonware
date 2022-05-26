// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdPerformanceValue::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdPerformanceValue::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdPerformanceValue::bdPerformanceValue()
    : bdTaskResult(), m_entityID(0), m_performanceValue(0)
{
}

bdPerformanceValue::~bdPerformanceValue()
{
}

bdBool bdPerformanceValue::deserialize(bdByteBufferRef buffer)
{
    bdBool ok = buffer->readUInt64(m_entityID);
    ok = ok == buffer->readInt64(m_performanceValue);
    if (!ok)
    {
        bdLogError("err", "Deserialization failed");
    }
    return ok;
}

bdUInt bdPerformanceValue::sizeOf()
{
    return sizeof(bdPerformanceValue);
}

void bdPerformanceValue::serialize(bdByteBuffer& buffer)
{
    buffer.writeUInt64(m_entityID);
    buffer.writeInt64(m_performanceValue);
}
