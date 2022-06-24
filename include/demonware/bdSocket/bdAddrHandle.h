// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdAddrHandle : public bdReferencable
{
public:
    enum bdAddrHandleStatus : bdInt
    {
        BD_ADDR_NOT_CONSTRUCTED = 0x0,
        BD_ADDR_NOT_RESOLVED = 0x1,
        BD_ADDR_ERROR = 0x2,
        BD_ADDR_RESOLVED = 0x3,
        BD_ADDR_UNRESOLVED = 0x4,
    };
public:
    bdEndpoint m_endpoint;
    bdAddrHandleStatus m_status;
    bdAddr m_realAddr;

    bdAddrHandle(bdCommonAddrRef ca, const bdSecurityID& secID);
    ~bdAddrHandle();
    void operator delete(void* p);
    bdBool operator==(const bdAddrHandle& other) const;
    void setRealAddr(const bdAddr& realAddr);
    void setStatus(const bdAddrHandleStatus& status);

    void getSecID(bdSecurityID& secID) const;
    bdInt getStatus() const;
    bdAddr getRealAddr() const;
};

typedef bdReference<bdAddrHandle> bdAddrHandleRef;
