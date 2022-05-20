// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

bdTag::bdTag(bdTag* other)
    : bdTaskResult(), m_priTag(other->m_priTag), m_secTag(other->m_secTag)
{
}

bdTag::bdTag(bdUInt64 priTag, bdUInt64 secTag)
    : bdTaskResult(), m_priTag(priTag), m_secTag(secTag)
{
}

bdTag::bdTag()
    : bdTaskResult(), m_priTag(0), m_secTag(0)
{
}

bdTag::~bdTag()
{
}

void bdTag::serialize(bdByteBuffer* buffer) const
{
    buffer->writeUInt64(m_priTag);
    buffer->writeUInt64(m_secTag);
}

bdBool bdTag::deserialize(bdByteBufferRef buffer)
{
    return buffer->readUInt64(&m_priTag) && buffer->readUInt64(&m_secTag);
}

bdUInt bdTag::sizeOf()
{
    return sizeof(bdTag);
}

void bdTag::set(bdUInt64 priTag, bdUInt64 secTag)
{
    m_priTag = priTag;
    m_secTag = secTag;
}

bdTag* bdTag::operator=(const bdTag* other)
{
    m_priTag = other->m_priTag;
    m_secTag = other->m_secTag;
    return this;
}
