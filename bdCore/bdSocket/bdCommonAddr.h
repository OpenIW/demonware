// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#define BD_COMMON_ADDR_SERIALIZED_SIZE 25
#define BD_MAX_LOCAL_ADDRS 3

enum bdNATType : __int32
{
    BD_NAT_UNKNOWN = 0x0,
    BD_NAT_OPEN = 0x1,
    BD_NAT_MODERATE = 0x2,
    BD_NAT_STRICT = 0x3,
};

class bdCommonAddr;
typedef bdReference<bdCommonAddr> bdCommonAddrRef;

class bdCommonAddr : bdReferencable
{
protected:
    bdArray<bdAddr> m_localAddrs;
    bdAddr m_publicAddr;
    bdNATType m_natType;
    bdUInt m_hash;
    bdBool m_isLoopback;
public:
    void* operator new(bdUWord nbytes);
    void operator delete(void* p);
    bdBool operator==(bdCommonAddr* other);
    bdBool operator!=(bdCommonAddr* other);
    bdBool operator<(bdCommonAddr* other);

    bdCommonAddr();
    bdCommonAddr(const bdAddr* publicAddr);
    bdCommonAddr(const bdArray<bdAddr>* localAddrs, const bdAddr* publicAddr, const bdNATType natType);
    bdCommonAddr(bdCommonAddrRef me, const bdArray<bdAddr>* localAddrs, const bdAddr* publicAddr, const bdNATType natType);
    ~bdCommonAddr();

    void calculateHash();
    bdBool deserialize(bdCommonAddrRef me, bdBitBufferRef buffer);
    bdBool deserialize(bdCommonAddrRef me, const bdUByte8* buffer);
    void serialize(bdBitBufferRef buffer);
    void serialize(bdUByte8* buffer);
    void setNATType(const bdNATType natType);

    const bdUInt getHash() const;
    const bdAddr* getLocalAddrByIndex(const bdUInt index) const;
    const bdArray<bdAddr>* getLocalAddrs() const;
    const bdNATType getNATType() const;
    const bdAddr* getPublicAddr() const;
    const bdBool isLoopback() const;
};
