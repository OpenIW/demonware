// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdAddressMap
{
protected:
    bdArray<bdReference<bdAddrHandle> > m_addrHandles;
public:
    bdAddressMap();
    ~bdAddressMap();
    bdBool getAddrHandle(const bdCommonAddrRef ca, const bdSecurityID* id, bdAddrHandleRef* addrHandle);
    bdBool findAddrHandle(const bdCommonAddrRef ca, const bdSecurityID* id, bdAddrHandleRef* addrHandle);
    bdBool addrToCommonAddr(const bdAddrHandleRef* addrHandle, bdCommonAddrRef* ca, bdSecurityID* id);
    bdBool unregisterAddr(bdAddrHandleRef* addrHandle);
    bdBool unregisterRealAddr(bdAddr* addr);
    bdUWord addrToString(const bdAddrHandleRef* addrHandle, bdNChar8* const str, const bdUWord size);
    void clear();
    bdBool commonAddrToAddr(const bdCommonAddrRef ca, const bdSecurityID* id, bdAddrHandleRef* addrHandle);
};