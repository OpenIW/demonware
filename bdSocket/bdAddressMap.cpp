// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocket.h"

bdAddressMap::bdAddressMap() : m_addrHandles(0u)
{
}

bdAddressMap::~bdAddressMap()
{
}

bdBool bdAddressMap::getAddrHandle(const bdCommonAddrRef ca, const bdSecurityID* id, bdAddrHandleRef* addrHandle)
{
    bdNChar8 publicIP[22];
    bdNChar8 localIP[22];
    bdBool found;
    bdUInt64 abAsNum;

    const_cast<bdAddr*>(ca.m_ptr->getLocalAddrByIndex(0))->getAddress()->toString(localIP, sizeof(localIP));
    const_cast<bdAddr*>(ca.m_ptr->getPublicAddr())->toString(publicIP, sizeof(publicIP));
    found = findAddrHandle(bdCommonAddrRef(ca), id, addrHandle);
    if (found)
    {
        return found;
    }
    bdMemcpy(&abAsNum, id, sizeof(abAsNum));
    bdLogInfo("bdSocket/socket",
        "getAddrHandle LocalIP: %s:%d Public IP: %s:%d id: %llu not found - creating!",
        localIP, const_cast<bdAddr*>(ca.m_ptr->getLocalAddrByIndex(0))->getPort(),
        publicIP, const_cast<bdAddr*>(ca.m_ptr->getPublicAddr())->getPort(), abAsNum);
    found = true;
    bdAddrHandle* p = new bdAddrHandle(bdCommonAddrRef(ca), id);
    // Check back later
    *addrHandle = p; // Why does addrHandle = p not work, when it exists as a operator... addrHandle->operator=(p) works but not the latter.
    m_addrHandles.pushBack(addrHandle);
    return found;
}

bdBool bdAddressMap::findAddrHandle(const bdCommonAddrRef ca, const bdSecurityID* id, bdAddrHandleRef* addrHandle)
{
    bdUInt numFound;
    bdUInt numAddrHandles;
    bdBool found;
    bdBool result = false;

    bdAssert(id != &bdSecurityID(), "Invalid security ID");
    if (ca.isNull())
    {
        bdLogError("bdSocket/bdAddressMap", "Invalid common addr reference.");
        return result;
    }
    bdEndpoint endpoint(bdCommonAddrRef(ca), id);
    numAddrHandles = m_addrHandles.getSize();
    numFound = 0;
    for (bdUInt i = 0; i < numAddrHandles; ++i)
    {
        found = false;
        bdAddrHandleRef handle(m_addrHandles[i]);
        if (handle.notNull())
        {
            found = handle->m_endpoint == &endpoint;
        }
        if (found)
        {
            if (!numFound)
            {
                result = true;
                addrHandle = &handle;
            }
            ++numFound;
        }
    }
    if (numFound > 1)
    {
        bdLogWarn("bdSocket/bdAddressMap", "Found %u matching address handles, expected 0 or 1.", numFound);
    }
    return result;
}

bdBool bdAddressMap::addrToCommonAddr(const bdAddrHandleRef* addrHandle, bdCommonAddrRef* ca, bdSecurityID* id)
{
    if (addrHandle->isNull())
    {
        bdLogError("bdSocket/bdAddressMap", "A invalid addr handle ref cannot be used.");
        return false;
    }
    ca = &bdAddrHandleRef(addrHandle)->m_endpoint.getCommonAddr();
    *id = bdAddrHandleRef(addrHandle)->m_endpoint.getSecID();
    if (ca->isNull())
    {
        bdLogWarn("bdSocket/bdAddressMap", "Addr handle ref has invalid common addr.");
    }
    return true;
}

bdBool bdAddressMap::unregisterAddr(bdAddrHandleRef* addrHandle)
{
    bdUInt numFound;
    bdBool found;

    if (addrHandle->isNull())
    {
        bdLogError("bdSocket/bdAddressMap", "A invalid addr handle ref cannot be unregistered.");
        return false;
    }
    found = false;
    numFound = 0;
    for (bdUInt i = 0; i < m_addrHandles.getSize(); ++i)
    {
        bdAddrHandleRef handle(m_addrHandles[i]);
        if (handle.notNull())
        {
            found = handle == addrHandle;
        }
        if (found)
        {
            m_addrHandles.removeAt(i--);
            ++numFound;
        }
    }
    if (numFound > 1)
    {
        bdLogWarn("bdSocket/bdAddressMap", "Found %u matching address handles, expected 0 or 1.", numFound);
    }
    if (!numFound)
    {
        bdLogWarn("bdSocket/bdAddressMap", "Failed to remove addr handle from address map.");
    }
    return true;
}

bdBool bdAddressMap::unregisterRealAddr(bdAddr* addr)
{
    bdNChar8 addrString[22];
    bdBool ok = false;

    for (bdUInt i = 0; i < m_addrHandles.getSize(); ++i)
    {
        bdAddrHandleRef handle(m_addrHandles[i]);
        bdBool found = false;
        if (handle.notNull())
        {
            found = handle->getRealAddr() == addr;
        }
        if (found)
        {
            bdAddrHandle::bdAddrHandleStatus status = bdAddrHandle::BD_ADDR_UNRESOLVED;
            handle->setStatus(&status);
            ok = true;
        }
    }
    if (!ok)
    {
        bdMemset(addrString, 0, sizeof(addrString));
        addr->toString(addrString, sizeof(addrString));
        bdLogWarn("bdSocket/bdAddressMap", "No address handles found matching the real address %s.", addrString);
    }
    return ok;
}

bdUWord bdAddressMap::addrToString(const bdAddrHandleRef* addrHandle, bdNChar8* const str, const bdUWord size)
{
    bdBool isLoopback;

    if (addrHandle->isNull())
    {
        bdStrlcpy(str, "Null addr handle ref", size);
        return bdStrlen("Null addr handle ref");
    }
    if ((*addrHandle)->getStatus() == bdAddrHandle::BD_ADDR_RESOLVED)
    {
        return (*addrHandle)->getRealAddr()->toString(str, size);
    }

    isLoopback = (*addrHandle)->m_endpoint.getCommonAddr()->isLoopback();
    if (isLoopback)
    {
        bdStrlcpy(str, "loopback", size);
        return bdStrlen("loopback");
    }
    bdStrlcpy(str, "not resolved", size);
    return bdStrlen("not resolved");
}

void bdAddressMap::clear()
{
    m_addrHandles.clear();
}

bdBool bdAddressMap::commonAddrToAddr(const bdCommonAddrRef ca, const bdSecurityID* id, bdAddrHandleRef* addrHandle)
{
    bdLogWarn("bdSocket/bdAddressMap", "This function is deprecated, use getAddrHandle instead. ");
    return getAddrHandle(bdCommonAddrRef(ca), id, addrHandle);
}
