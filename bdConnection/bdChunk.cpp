// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdConnection/bdConnection.h"

bdUInt bdChunk::serialize(bdUByte8* data, const bdUInt32 size) const
{
    bdUInt offset = 0;
    bdBytePacker::appendBasicType<bdUByte8>(data, size, 0u, offset, m_type);
    return offset;
}

bdBool bdChunk::deserialize(const bdUByte8* const data, const bdUInt size, bdUInt& offset)
{
    if (size - offset > 4)
    {
        bdUByte8 type = 0;
        return bdBytePacker::removeBasicType<bdUByte8>(data, size, offset, offset, type);
        m_type = static_cast<bdChunkTypes>(type);
    }
    return false;
}

bdUInt bdChunk::getSerializedSize()
{
    return serialize(NULL, NULL);
}

const bdChunkTypes bdChunk::getType() const
{
    return m_type;
}

const bdBool bdChunk::isControl() const
{
    return m_type != 2;
}

bdChunkTypes bdChunk::getType(const bdUByte8* const data, const bdUInt size)
{
    bdUInt offset = 0;
    bdUByte8 tmp = 0;
    bdBool ok = bdBytePacker::removeBasicType<bdUByte8>(data, size, 0, offset, tmp);
    if (ok)
    {
        return static_cast<bdChunkTypes>(tmp);
    }
    return BD_CT_INVALID;
}
