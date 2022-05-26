// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdConnection/bdConnection.h"

void bdMessage::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdMessage::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdMessage::bdMessage(const bdMessageType type, const bdBool typeChecked)
    : bdReferencable(), m_type(type), m_payload(), m_payloadTypeChecked(typeChecked), m_unencPayload()
{
}

bdMessage::bdMessage(const bdMessageType type, const bdUByte8* const payload, const bdUInt32 size, const bdBool hasTypeCheckedBit, const bdUByte8* const unencPayload, const bdUInt32 unencSize)
    : bdReferencable(), m_type(type), m_payload(), m_payloadTypeChecked(false), m_unencPayload()
{
    if (payload && size)
    {
        m_payload = new bdBitBuffer(payload, 8 * size, hasTypeCheckedBit);
        m_payloadTypeChecked = m_payload->getTypeCheck();
    }
    if (unencPayload && unencSize)
    {
        m_unencPayload = new bdByteBuffer(unencSize, false);
        m_unencPayload->write(unencPayload, unencSize);
    }
}

void bdMessage::createUnencryptedPayload(const bdUInt size)
{
    m_unencPayload = new bdByteBuffer(size, 0);
}

bdBitBufferRef bdMessage::getPayload()
{
    if (m_payload.isNull())
    {
        m_payload = new bdBitBuffer(0, m_payloadTypeChecked);
    }
    return bdBitBufferRef(m_payload);
}

const bdByteBufferRef bdMessage::getUnencryptedPayload() const
{
    return bdByteBufferRef(m_unencPayload);
}

const bdBool bdMessage::hasUnencryptedPayload() const
{
    return m_unencPayload.notNull();
}

const bdBool bdMessage::hasPayload() const
{
    return m_payload.notNull();
}

const bdMessageType bdMessage::getType() const
{
    return m_type;
}
