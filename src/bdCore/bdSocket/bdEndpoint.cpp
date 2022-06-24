// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdCore/bdCore.h"

bdEndpoint::bdEndpoint()
{
}

bdEndpoint::bdEndpoint(const bdEndpoint& other)
{
    m_ca = bdCommonAddrRef(other.m_ca);
    m_secID = bdSecurityID(other.m_secID);
}

bdEndpoint::bdEndpoint(const bdCommonAddrRef addr, const bdSecurityID& secID)
{
    m_ca = bdCommonAddrRef(*addr);
    m_secID = bdSecurityID(secID);
}

bdEndpoint::~bdEndpoint()
{
}

bdBool bdEndpoint::operator==(const bdEndpoint& other) const
{
    if (m_ca.notNull() && other.m_ca.notNull())
    {
        if (m_ca->getHash() == other.m_ca->getHash())
        {
            return m_secID == other.m_secID;
        }
        return false;
    }
    else if (m_ca.isNull() && other.m_ca.isNull())
    {
        return true;
    }
    return false;
}

bdCommonAddrRef bdEndpoint::getCommonAddr() const
{
    return bdCommonAddrRef(m_ca);
}

const bdSecurityID& bdEndpoint::getSecID() const
{
    return m_secID;
}

const bdUInt bdEndpoint::getHash() const
{
    bdUInt hash = 0;

    if (bdCommonAddrRef(m_ca).notNull())
    {
        hash = bdCommonAddrRef(m_ca)->getHash();
        bdHashingClass hasher;
        hash += hasher.getHash<bdSecurityID>(getSecID());
    }
    return hash;
}

bdUInt bdEndpoint::getSerializedLength() const
{
    return BD_ENDPOINT_SERIALIZED_SIZE;
}

bdBool bdEndpoint::serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset)
{
    newOffset = offset;
    bdAssert(offset <= size, "Offset is past the end of the destination buffer.");
    if (getSerializedLength() > size - offset)
    {
        newOffset = offset;
        return false;
    }
    m_ca->serialize((bdUByte8*)data + newOffset);
    newOffset += (sizeof(bdCommonAddr) + 1);
    bdMemcpy((bdUByte8*)data + newOffset, m_secID.ab, sizeof(m_secID.ab));
    newOffset += sizeof(m_secID.ab);
    return true;
}

bdBool bdEndpoint::deserialize(bdCommonAddrRef me, const void* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset)
{
    newOffset = offset;

    if (newOffset + (sizeof(bdCommonAddr) + 1) > size)
    {
        newOffset = offset;
        return false;
    }
    if (m_ca.isNull())
    {
        m_ca = new bdCommonAddr();
    }
    m_ca->deserialize(*me, (const bdUByte8*)data + newOffset);
    newOffset += (sizeof(bdCommonAddr) + 1);
    if (newOffset + sizeof(bdSecurityID) > size)
    {
        newOffset = offset;
        return false;
    }
    bdMemcpy(m_secID.ab, (bdUByte8*)data + newOffset, sizeof(m_secID.ab));
    return true;
}

bdUInt bdEndpointHashingClass::getHash(const bdEndpoint& other)
{
    return other.getHash();
}
