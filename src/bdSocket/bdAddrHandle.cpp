// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocket.h"

bdAddrHandle::bdAddrHandle(bdCommonAddrRef ca, const bdSecurityID& secID)
    : bdReferencable(), m_endpoint(bdCommonAddrRef(ca), secID), m_status(BD_ADDR_NOT_RESOLVED), m_realAddr()
{
}

bdAddrHandle::~bdAddrHandle()
{
}

void bdAddrHandle::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

bdBool bdAddrHandle::operator==(const bdAddrHandle& other) const
{
    return m_endpoint == other.m_endpoint;
}

void bdAddrHandle::setRealAddr(const bdAddr& realAddr)
{
    bdMemcpy(&m_realAddr, &realAddr, sizeof(m_realAddr));
    m_status = BD_ADDR_RESOLVED;
}

void bdAddrHandle::setStatus(const bdAddrHandleStatus& status)
{
    m_status = status;
}

void bdAddrHandle::getSecID(bdSecurityID& secID) const
{
    secID = m_endpoint.getSecID();
}

bdInt bdAddrHandle::getStatus() const
{
    return m_status;
}

bdAddr bdAddrHandle::getRealAddr() const
{
    return bdAddr(m_realAddr);
}
