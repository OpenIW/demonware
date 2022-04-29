// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdNet/bdNet.h"

void* bdGameInfo::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

void bdGameInfo::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

bdGameInfo::bdGameInfo()
    : bdReferencable(), m_titleId(666), m_secID(), m_secKey(), m_hostAddr()
{
}

bdGameInfo::bdGameInfo(bdUInt titleID, const bdSecurityID* securityID, const bdSecurityKey* securityKey, bdCommonAddrRef hostAddr)
    : bdReferencable(), m_titleId(titleID), m_secID(securityID), m_secKey(securityKey), m_hostAddr(hostAddr)
{
}

bdGameInfo::~bdGameInfo()
{
}

void bdGameInfo::serialize(bdBitBuffer* bitBuffer)
{
    bdUByte8 hostAddrBuffer[25];

    if (m_titleId == 666)
    {
        bdLogWarn("discovery/gameinfo", "This object appears to be uninitialzied.");
    }
    if (m_hostAddr.notNull())
    {
        bitBuffer->writeDataType(BD_BB_UNSIGNED_INTEGER32_TYPE);
        bitBuffer->writeBits(&m_titleId, CHAR_BIT * sizeof(m_titleId));
        bitBuffer->writeDataType(BD_BB_FULL_TYPE);
        bitBuffer->writeBits(&m_secID, CHAR_BIT * sizeof(m_secID));
        bitBuffer->writeDataType(BD_BB_FULL_TYPE);
        bitBuffer->writeBits(&m_secKey, CHAR_BIT * sizeof(m_secKey));
        m_hostAddr->serialize(hostAddrBuffer);
        bitBuffer->writeBits(hostAddrBuffer, 200u); // Check later
    }
    else
    {
        bdLogError("discovery/gameinfo", "Serialized failed as data is uninitialized.");
    }
}

bdBool bdGameInfo::deserialize(const bdCommonAddrRef localAddr, bdBitBuffer* bitBuffer)
{
    return bdBool();
}

void bdGameInfo::setHostAddr(const bdCommonAddrRef hostAddr)
{
}

void bdGameInfo::setSecurityID(const bdSecurityID* secID)
{
}

void bdGameInfo::setSecurityKey(const bdSecurityKey* secKey)
{
}

void bdGameInfo::setTitleID(const bdUInt titleID)
{
}

const bdUInt bdGameInfo::getTitleID() const
{
    if (m_titleId == 666)
    {
        bdLogWarn("discovery/gameinfo", "This object appears to be uninitialized.");
    }
    return m_titleId;
}

const bdSecurityKey* bdGameInfo::getSecurityKey() const
{
    return &m_secKey;
}

const bdSecurityID* bdGameInfo::getSecurityID() const
{
    return &m_secID;
}

const bdCommonAddrRef* bdGameInfo::getHostAddr() const
{
    return &bdCommonAddrRef(m_hostAddr);
}
