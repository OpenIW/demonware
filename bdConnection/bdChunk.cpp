// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdConnection/bdConnection.h"

bdUInt bdChunk::serialize(bdUByte8* data, const bdUInt32 size)
{
    bdUInt offset = 0;
    bdBytePacker::appendBasicType<bdUByte8>(data, size, 0u, &offset, reinterpret_cast<bdUByte8*>(&m_type));
    return offset;
}

bdBool bdChunk::deserialize(const bdUByte8* const data, const bdUInt size, bdUInt* offset)
{
    if (size - *offset > 4)
    {
        return bdBytePacker::removeBasicType<bdUByte8>(data, size, *offset, offset, reinterpret_cast<bdUByte8*>(&m_type));
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
