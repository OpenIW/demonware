// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocket.h"

bdUInt bdQoSReplyPacket::headerSize = 0;

bdQoSReplyPacket::bdQoSReplyPacket() : m_type(41), m_id(0), m_timestamp(0), m_enabledMode(true), m_data(NULL), m_dataSize(0), m_serializedSize(0)
{
}

bdQoSReplyPacket::~bdQoSReplyPacket()
{
    if (m_data)
    {
        bdMemory::deallocate(m_data);
        m_data = NULL;
        m_dataSize = 0;
    }
}

bdUByte8 bdQoSReplyPacket::getType() const
{
    return m_type;
}

bdUInt32 bdQoSReplyPacket::getId() const
{
    return m_id;
}

bdUInt64 bdQoSReplyPacket::getTimestamp() const
{
    return m_timestamp;
}

bdBool bdQoSReplyPacket::getEnabled() const
{
    return m_enabledMode;
}

bdUByte8* bdQoSReplyPacket::getData() const
{
    return m_data;
}

bdUInt32 bdQoSReplyPacket::getDataSize() const
{
    return m_dataSize;
}

bdUInt bdQoSReplyPacket::getSerializedSize() const
{
    return m_serializedSize;
}

bdUInt bdQoSReplyPacket::getHeaderSize()
{
    bdUInt oldDataSize;

    if (!headerSize)
    {
        oldDataSize = m_dataSize;
        m_dataSize = 0;
        serialize(0, 65535, 0, headerSize);
        m_dataSize = oldDataSize;
    }
    return headerSize;
}

bdBool bdQoSReplyPacket::setData(const bdUByte8* data, bdUInt32 dataSize)
{
    if (dataSize > 1288 - getHeaderSize())
    {
        bdLogWarn("bdSocket/qos", "Attemped to set more data then available.");
        return false;
    }
    if (m_data)
    {
        bdDeallocate<bdUByte8>(m_data);
        m_data = NULL;
        m_dataSize = 0;
        m_serializedSize = 0;
    }
    if (!dataSize)
    {
        return false;
    }
    m_data = bdAllocate<bdUByte8>(dataSize);
    if (!m_data)
    {
        bdLogWarn("bdSocket/qos", "Failed to allocate data buffer.");
        return false;
    }
    m_dataSize = dataSize;
    bdMemcpy(m_data, data, dataSize);
    return true;
}

void bdQoSReplyPacket::setEnabled(bdBool mode)
{
    m_enabledMode = mode;
}

void bdQoSReplyPacket::setId(bdUInt32 id)
{
    m_id = id;
}

void bdQoSReplyPacket::setTimestamp(bdUInt64 timestamp)
{
    m_timestamp = timestamp;
}

bdBool bdQoSReplyPacket::deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset)
{
    bdBool ok;

    newOffset = offset;
    ok = bdBytePacker::removeBasicType<bdUByte8>(data, size, newOffset, newOffset, m_type);
    ok = ok == bdBytePacker::removeBasicType<bdUInt32>(data, size, newOffset, newOffset, m_id);
    ok = ok == bdBytePacker::removeBasicType<bdUInt64>(data, size, newOffset, newOffset, m_timestamp);
    ok = ok == bdBytePacker::removeBasicType<bdBool>(data, size, newOffset, newOffset, m_enabledMode);

    if (size - newOffset != m_dataSize)
    {
        bdLogWarn("bdSocket/qos", "Received a packet with invalid dataSize!");
        ok = false;
    }
    if (ok && m_data)
    {
        bdDeallocate<bdUByte8>(m_data);
        m_data = NULL;
    }
    if (ok && m_dataSize)
    {
        m_data = bdAllocate<bdUByte8>(m_dataSize);
        if (!m_data)
        {
            bdLogWarn("bdSocket/qos", "Failed to allocate data buffer.");
            ok = false;
        }
        ok = ok == bdBytePacker::removeBuffer(reinterpret_cast<const bdUByte8*>(data), size, newOffset, newOffset, m_data, m_dataSize);
    }
    if (!ok)
    {
        newOffset = offset;
    }
    return ok;
}

bdBool bdQoSReplyPacket::serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset) const
{
    bdBool ok;

    newOffset = offset;
    ok = bdBytePacker::appendBasicType<bdUByte8>(data, size, newOffset, newOffset, m_type);
    ok = ok == bdBytePacker::appendBasicType<bdUInt32>(data, size, newOffset, newOffset, m_id);
    ok = ok == bdBytePacker::appendBasicType<bdUInt64>(data, size, newOffset, newOffset, m_timestamp);
    ok = ok == bdBytePacker::appendBasicType<bdBool>(data, size, newOffset, newOffset, m_enabledMode);
    ok = ok == bdBytePacker::appendBasicType<bdUInt32>(data, size, newOffset, newOffset, m_dataSize);
    if (m_dataSize)
    {
        ok = ok == bdBytePacker::appendBuffer(reinterpret_cast<bdUByte8*>(data), size, newOffset, newOffset, m_data, m_dataSize);
    }
    if (!ok)
    {
        newOffset = offset;
    }
    return ok;
}
