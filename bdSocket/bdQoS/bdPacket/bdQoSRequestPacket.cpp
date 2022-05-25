// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocket.h"

bdQoSRequestPacket::bdQoSRequestPacket() : m_type(0), m_timestamp(0), m_id(0), m_secid(0)
{

}

bdQoSRequestPacket::bdQoSRequestPacket(bdUInt32 secid, bdUInt32 id) : m_type(40), m_id(id), m_secid(secid), m_timestamp(bdPlatformTiming::getHiResTimeStamp().QuadPart)
{
}

bdUInt32 bdQoSRequestPacket::getId() const
{
    return m_id;
}

bdUInt64 bdQoSRequestPacket::getTimestamp() const
{
    return m_timestamp;
}

bdUByte8 bdQoSRequestPacket::getType() const
{
    return m_type;
}

bdUInt32 bdQoSRequestPacket::getSecId() const
{
    return m_secid;
}

bdBool bdQoSRequestPacket::deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset)
{
    bdBool ok;

    newOffset = offset;
    ok = bdBytePacker::removeBasicType<bdUByte8>(data, size, newOffset, newOffset, &m_type);
    ok = ok == bdBytePacker::removeBasicType<bdUInt64>(data, size, newOffset, newOffset, &m_timestamp);
    ok = ok == bdBytePacker::removeBasicType<bdUInt32>(data, size, newOffset, newOffset, &m_id);
    ok = ok == bdBytePacker::removeBasicType<bdUInt32>(data, size, newOffset, newOffset, &m_secid);
    if (!ok)
    {
        newOffset = offset;
    }
    return ok;
}

bdBool bdQoSRequestPacket::serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset) const
{
    bdBool ok;

    newOffset = offset;
    ok = bdBytePacker::appendBasicType<bdUByte8>(data, size, newOffset, newOffset, &m_type);
    ok = ok == bdBytePacker::appendBasicType<bdUInt64>(data, size, newOffset, newOffset, &m_timestamp);
    ok = ok == bdBytePacker::appendBasicType<bdUInt32>(data, size, newOffset, newOffset, &m_id);
    ok = ok == bdBytePacker::appendBasicType<bdUInt32>(data, size, newOffset, newOffset, &m_secid);
    if (!ok)
    {
        newOffset = offset;
    }
    return ok;
}
