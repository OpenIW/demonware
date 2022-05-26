// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdCore/bdCore.h"

void* bdCommonAddr::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

void bdCommonAddr::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

bdBool bdCommonAddr::operator==(bdCommonAddr& other) const
{
    bdAddr localAddr = m_localAddrs.getSize() ? bdAddr(m_localAddrs[0]) : bdAddr();
    bdAddr otherLocalAddr = other.m_localAddrs.getSize() ? bdAddr(other.m_localAddrs[0]) : bdAddr();

    if (m_hash == other.m_hash)
    {
        bdAddr myAddr = m_publicAddr.getAddress().isValid() ? m_publicAddr : localAddr;
        return other.m_publicAddr.getAddress().isValid() ? myAddr == other.m_publicAddr : myAddr == otherLocalAddr;
    }
    return false;
}

bdBool bdCommonAddr::operator!=(bdCommonAddr& other) const
{
    return m_hash != other.m_hash;
}

bdBool bdCommonAddr::operator<(bdCommonAddr& other) const
{
    return m_hash < other.m_hash;
}

bdCommonAddr::bdCommonAddr() : bdReferencable()
{
    m_localAddrs = bdArray<bdAddr>(0u);
    m_publicAddr = bdAddr();
    m_natType = BD_NAT_OPEN;
    m_hash = 0;
    m_isLoopback = false;
}

bdCommonAddr::bdCommonAddr(const bdAddr& publicAddr) : bdReferencable()
{
    m_localAddrs = bdArray<bdAddr>((bdUInt)0);
    m_publicAddr = bdAddr(publicAddr);
    m_natType = BD_NAT_OPEN;
    m_isLoopback = false;
    bdAssert(publicAddr.getAddress().isValid(), "Address not valid!");
    m_localAddrs.pushBack(publicAddr);
    calculateHash();
}

bdCommonAddr::bdCommonAddr(const bdArray<bdAddr>& localAddrs, const bdAddr& publicAddr, const bdNATType natType) : bdReferencable()
{
    m_localAddrs = bdArray<bdAddr>(localAddrs);
    m_publicAddr = bdAddr(publicAddr);
    m_natType = natType;
    m_isLoopback = true;
    bdAssert(localAddrs.getSize() > 0, "Too few local addresses!");
    bdAssert(localAddrs.getSize() <= BD_MAX_LOCAL_ADDRS, "Too many local addresses!");
    calculateHash();
}

bdCommonAddr::bdCommonAddr(bdCommonAddrRef me, const bdArray<bdAddr>& localAddrs, const bdAddr& publicAddr, const bdNATType natType) : bdReferencable()
{
    m_localAddrs = bdArray<bdAddr>(localAddrs);
    m_publicAddr = bdAddr(publicAddr);
    m_natType = natType;
    m_isLoopback = false;
    bdAssert(localAddrs.getSize() > 0, "Too few local addresses!");
    bdAssert(localAddrs.getSize() <= BD_MAX_LOCAL_ADDRS, "Too many local addresses!");
    calculateHash();
    if (*me)
    {
        if (*me == this)
        {
            m_isLoopback = true;
        }
    }
}

bdCommonAddr::~bdCommonAddr()
{
}

void bdCommonAddr::calculateHash()
{
    bdBool status;
    bdUInt tmpHashSize;
    bdUInt offset;
    bdUByte8 data[6];
    bdUByte8 hash[24];
    bdAddr addr;
    bdAddr localAddr = m_localAddrs.getSize() ? bdAddr(m_localAddrs[0]) : bdAddr();

    if (m_publicAddr.getAddress().isValid())
    {
        bdMemcpy(&addr, &m_publicAddr, sizeof(m_publicAddr));
    }
    else
    {
        bdMemcpy(&addr, &localAddr, sizeof(localAddr));
    }
    offset = 0;
    status = addr.serialize(data, sizeof(data), 0, offset);
    tmpHashSize = 24;

    bdHashTiger192 tiger = bdHashTiger192();
    if (status)
    {
        status = tiger.hash(data, sizeof(data), hash, tmpHashSize);
    }
    if (status)
    {
        status = bdBytePacker::removeBasicType<bdUInt>(hash, sizeof(hash), 0, offset, m_hash);
    }
    bdAssert(status, "Failed to calculate hash.");
}

bdBool bdCommonAddr::deserialize(bdCommonAddrRef me, bdBitBufferRef buffer)
{
    bdUByte8 tmpBuffer[BD_COMMON_ADDR_SERIALIZED_SIZE];

    if (buffer.m_ptr->readBits(tmpBuffer, BD_COMMON_ADDR_SERIALIZED_SIZE * 8))
    {
        return deserialize(me.m_ptr, tmpBuffer);
    }
    return false;
}

bdBool bdCommonAddr::deserialize(bdCommonAddrRef me, const bdUByte8* buffer)
{
    bdUInt offset = 0;
    bdBool status = true;
    bdCommonAddr* other = this;

    m_localAddrs.clear();
    for (bdUInt i = 0; i < BD_MAX_LOCAL_ADDRS; ++i)
    {
        bdAddr localAddr = bdAddr();
        status = status == localAddr.deserialize(buffer, BD_COMMON_ADDR_SERIALIZED_SIZE, offset, offset);
        if (status)
        {
            if (localAddr.getAddress().isValid())
            {
                other->m_localAddrs.pushBack(localAddr);
            }
        }
    }
    status = status == other->m_publicAddr.deserialize(buffer, BD_COMMON_ADDR_SERIALIZED_SIZE, offset, offset);
    bdByte8 tmpByte = 0;
    status = status == bdBytePacker::removeBasicType<bdByte8>(buffer, BD_COMMON_ADDR_SERIALIZED_SIZE, offset, offset, tmpByte);
    if (status)
    {
        other->m_natType = static_cast<bdNATType>(tmpByte);
        other->calculateHash();
        other->m_isLoopback = (me.m_ptr == other);
    }
    return status;
}

const bdUInt bdCommonAddr::getHash() const
{
    return m_hash;
}

const bdAddr& bdCommonAddr::getLocalAddrByIndex(const bdUInt index) const
{
    return m_localAddrs[index];
}

const bdArray<bdAddr>& bdCommonAddr::getLocalAddrs() const
{
    return m_localAddrs;
}

const bdNATType bdCommonAddr::getNATType() const
{
    return m_natType;
}

const bdAddr& bdCommonAddr::getPublicAddr() const
{
    return m_publicAddr;
}

const bdBool bdCommonAddr::isLoopback() const
{
    return m_isLoopback;
}

void bdCommonAddr::serialize(bdBitBufferRef buffer)
{
    bdUByte8 tmpBuffer[BD_COMMON_ADDR_SERIALIZED_SIZE];

    serialize(tmpBuffer);
    buffer.m_ptr->writeBits(tmpBuffer, BD_COMMON_ADDR_SERIALIZED_SIZE * 8);
}

void bdCommonAddr::serialize(bdUByte8* buffer)
{
    bdUInt offset = 0;
    bdBool status = true;
    bdAddr invalidAddr;
    bdNChar8 var;

    for (bdUInt i = 0; i < BD_MAX_LOCAL_ADDRS; ++i)
    {
        if (i >= m_localAddrs.getSize())
        {
            status = status == invalidAddr.serialize(buffer, BD_COMMON_ADDR_SERIALIZED_SIZE, offset, offset);
        }
        else
        {
            status = status == m_localAddrs[i].serialize(buffer, BD_COMMON_ADDR_SERIALIZED_SIZE, offset, offset);
        }
    }
    status = status == m_publicAddr.serialize(buffer, BD_COMMON_ADDR_SERIALIZED_SIZE, offset, offset);
    if (status)
    {
        var = m_natType;
        status = bdBytePacker::appendBasicType<bdNChar8>(buffer, BD_COMMON_ADDR_SERIALIZED_SIZE, offset, offset, var);
    }
    bdAssert(status && offset == BD_COMMON_ADDR_SERIALIZED_SIZE, "bdCommonAddr::serialize, wrong size.");
}

void bdCommonAddr::setNATType(const bdNATType natType)
{
    if (natType < BD_NAT_UNKNOWN || natType > BD_NAT_STRICT)
    {
        bdLogWarn("warn", "Attempting to set invalid NAT type of %d. Ignoring.", natType);
        return;
    }
    m_natType = natType;
    bdLogWarn("warn", "Overriding NAT type of bdCommonAddr. This should be done for testing purposes only!");
}
