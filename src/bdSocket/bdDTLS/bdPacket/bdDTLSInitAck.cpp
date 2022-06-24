// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocket.h"

void bdDTLSInitAck::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

bdDTLSInitAck::bdDTLSInitAck()
    : bdDTLSHeader(), m_timestamp(0), m_signature(0), m_initTag(0), m_localTag(0), m_peerTag(0), m_localTieTag(0), m_peerTieTag(0), m_peerAddr(), m_secID()
{
}

bdDTLSInitAck::bdDTLSInitAck(bdUInt16 vtag, bdUInt16 initTag, bdUInt16 localTag, bdUInt16 peerTag, bdUInt16 localTieTag, bdUInt16 peerTieTag, bdUInt32 timestamp, const bdAddr& peerAddr, bdSecurityID secID)
    : bdDTLSHeader(BD_DTLS_INIT_ACK, vtag, 0), m_timestamp(timestamp), m_signature(0), m_initTag(initTag), m_localTag(localTag), m_peerTag(peerTag), m_localTieTag(localTieTag), m_peerTieTag(peerTieTag),
    m_peerAddr(peerAddr), m_secID(secID)
{
}

bdDTLSInitAck::bdDTLSInitAck(const bdDTLSInitAck& other)
    : bdDTLSHeader(static_cast<bdDTLSPacketTypes>(other.m_type), other.m_vtag, other.m_counter), m_timestamp(other.m_timestamp), m_signature(other.m_signature), m_initTag(other.m_initTag),
    m_localTag(other.m_localTag), m_peerTag(other.m_peerTag), m_localTieTag(other.m_localTieTag), m_peerTieTag(other.m_peerTieTag), m_peerAddr(other.m_peerAddr), m_secID(other.m_secID)
{
}

bdDTLSInitAck* bdDTLSInitAck::operator=(bdDTLSInitAck& other)
{
    m_type = other.m_type;
    m_version = other.m_version;
    m_vtag = other.m_vtag;
    m_counter = other.m_counter;
    m_timestamp = other.m_timestamp;
    m_signature = other.m_signature;
    m_initTag = other.m_initTag;
    m_localTag = other.m_localTag;
    m_peerTag = other.m_peerTag;
    m_localTieTag = other.m_localTieTag;
    m_peerTieTag = other.m_peerTieTag;
    m_peerAddr = other.m_peerAddr;
    m_secID = other.m_secID;
    return this;
}

bdDTLSInitAck::~bdDTLSInitAck()
{
}

bdBool bdDTLSInitAck::serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset)
{
    bdBool ok;

    newOffset = offset;
    ok = bdDTLSHeader::serialize(data, size, newOffset, newOffset);
    ok = ok == bdBytePacker::appendBasicType<bdUInt>(data, size, newOffset, newOffset, m_timestamp);
    ok = ok == bdBytePacker::appendBasicType<bdUInt>(data, size, newOffset, newOffset, m_signature);
    ok = ok == bdBytePacker::appendBasicType<bdUInt16>(data, size, newOffset, newOffset, m_initTag);
    ok = ok == bdBytePacker::appendBasicType<bdUInt16>(data, size, newOffset, newOffset, m_localTag);
    ok = ok == bdBytePacker::appendBasicType<bdUInt16>(data, size, newOffset, newOffset, m_peerTag);
    ok = ok == bdBytePacker::appendBasicType<bdUInt16>(data, size, newOffset, newOffset, m_localTieTag);
    ok = ok == bdBytePacker::appendBasicType<bdUInt16>(data, size, newOffset, newOffset, m_peerTieTag);
    ok = ok == m_peerAddr.serialize(reinterpret_cast<bdUByte8*>(data), size, newOffset, newOffset);
    ok = ok == bdBytePacker::appendBuffer(reinterpret_cast<bdUByte8*>(data), size, newOffset, newOffset, &m_secID, sizeof(bdSecurityID));
    if (!ok)
    {
        newOffset = offset;
    }
    return ok;
}

bdBool bdDTLSInitAck::deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset)
{
    bdBool ok;

    newOffset = offset;
    ok = bdDTLSHeader::deserialize(data, size, newOffset, newOffset);
    ok = ok == bdBytePacker::removeBasicType<bdUInt>(data, size, newOffset, newOffset, m_timestamp);
    ok = ok == bdBytePacker::removeBasicType<bdUInt>(data, size, newOffset, newOffset, m_signature);
    ok = ok == bdBytePacker::removeBasicType<bdUInt16>(data, size, newOffset, newOffset, m_initTag);
    ok = ok == bdBytePacker::removeBasicType<bdUInt16>(data, size, newOffset, newOffset, m_localTag);
    ok = ok == bdBytePacker::removeBasicType<bdUInt16>(data, size, newOffset, newOffset, m_peerTag);
    ok = ok == bdBytePacker::removeBasicType<bdUInt16>(data, size, newOffset, newOffset, m_localTieTag);
    ok = ok == bdBytePacker::removeBasicType<bdUInt16>(data, size, newOffset, newOffset, m_peerTieTag);
    ok = ok == m_peerAddr.deserialize(reinterpret_cast<const bdUByte8*>(data), size, newOffset, newOffset);
    ok = ok == bdBytePacker::removeBuffer(reinterpret_cast<const bdUByte8*>(data), size, newOffset, newOffset, &m_secID, sizeof(bdSecurityID));
    if (!ok)
    {
        newOffset = offset;
    }
    return ok;
}

void bdDTLSInitAck::sign(bdHMacSHA1& hmac)
{
    bdUInt tmpAddrBufferUsedSize;
    bdUByte8 tmpAddrBuffer[sizeof(bdAddr)];
    bdUInt sigsize;

    m_signature = 0;
    sigsize = sizeof(m_signature);
    hmac.process(reinterpret_cast<bdUByte8*>(&m_timestamp), sizeof(m_timestamp));
    hmac.process(reinterpret_cast<bdUByte8*>(&m_initTag), sizeof(m_initTag));
    hmac.process(reinterpret_cast<bdUByte8*>(&m_localTag), sizeof(m_localTag));
    hmac.process(reinterpret_cast<bdUByte8*>(&m_peerTag), sizeof(m_peerTag));
    hmac.process(reinterpret_cast<bdUByte8*>(&m_localTieTag), sizeof(m_localTieTag));
    hmac.process(reinterpret_cast<bdUByte8*>(&m_peerTieTag), sizeof(m_peerTieTag));
    tmpAddrBufferUsedSize = 0;
    m_peerAddr.serialize(tmpAddrBuffer, sizeof(bdAddr), 0, tmpAddrBufferUsedSize);
    hmac.process(tmpAddrBuffer, tmpAddrBufferUsedSize);
    hmac.getData(reinterpret_cast<bdUByte8*>(&m_signature), sigsize);
}

bdBool bdDTLSInitAck::verify(bdHMacSHA1& hmac)
{
    bdUInt tmpAddrBufferUsedSize;
    bdUByte8 tmpAddrBuffer[sizeof(bdAddr)];
    bdUInt psign;
    bdUInt sigsize;

    sigsize = sizeof(psign);
    hmac.process(reinterpret_cast<bdUByte8*>(&m_timestamp), sizeof(m_timestamp));
    hmac.process(reinterpret_cast<bdUByte8*>(&m_initTag), sizeof(m_initTag));
    hmac.process(reinterpret_cast<bdUByte8*>(&m_localTag), sizeof(m_localTag));
    hmac.process(reinterpret_cast<bdUByte8*>(&m_peerTag), sizeof(m_peerTag));
    hmac.process(reinterpret_cast<bdUByte8*>(&m_localTieTag), sizeof(m_localTieTag));
    hmac.process(reinterpret_cast<bdUByte8*>(&m_peerTieTag), sizeof(m_peerTieTag));
    tmpAddrBufferUsedSize = 0;
    m_peerAddr.serialize(tmpAddrBuffer, sizeof(bdAddr), 0, tmpAddrBufferUsedSize);
    hmac.process(tmpAddrBuffer, tmpAddrBufferUsedSize);
    hmac.getData(reinterpret_cast<bdUByte8*>(&psign), sigsize);
    return m_signature == psign;
}

const bdUInt16 bdDTLSInitAck::getLocalTag() const
{
    return m_localTag;
}

const bdUInt16 bdDTLSInitAck::getPeerTag() const
{
    return m_peerTag;
}

const bdUInt16 bdDTLSInitAck::getLocalTieTag() const
{
    return m_localTieTag;
}

const bdUInt16 bdDTLSInitAck::getPeerTieTag() const
{
    return m_peerTieTag;
}

const bdUInt16 bdDTLSInitAck::getInitTag() const
{
    return m_initTag;
}

const bdAddr& bdDTLSInitAck::getPeerAddr() const
{
    return m_peerAddr;
}

const bdUInt bdDTLSInitAck::getTimestamp() const
{
    return m_timestamp;
}

void bdDTLSInitAck::getSecID(bdSecurityID& secID) const
{
    secID = m_secID;
}
