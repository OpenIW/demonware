// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdGameInfo : bdReferencable
{
protected:
    bdUInt m_titleId;
    bdSecurityID m_secID;
    bdSecurityKey m_secKey;
    bdCommonAddrRef m_hostAddr;
public:
    void* operator new(bdUWord nbytes);
    void operator delete(void* p);
    bdGameInfo();
    bdGameInfo(bdUInt titleID, const bdSecurityID* securityID, const bdSecurityKey* securityKey, bdCommonAddrRef hostAddr);
    ~bdGameInfo();
    virtual void serialize(bdBitBuffer* bitBuffer);
    virtual bdBool deserialize(const bdCommonAddrRef localAddr, bdBitBuffer* bitBuffer);

    void setHostAddr(const bdCommonAddrRef hostAddr);
    void setSecurityID(const bdSecurityID* secID);
    void setSecurityKey(const bdSecurityKey* secKey);
    void setTitleID(const bdUInt titleID);

    const bdUInt getTitleID() const;
    const bdSecurityKey* getSecurityKey() const;
    const bdSecurityID* getSecurityID() const;
    const bdCommonAddrRef* getHostAddr() const;
};
