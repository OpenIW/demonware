// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdAddr
{
protected:
    bdInetAddr m_address;
    bdPort m_port;
public:
    static bdUInt serializedSize;

    void* operator new(bdUWord nbytes, void* p);
    void* operator new(bdUWord nbytes);
    void operator delete(void* p);
    bdAddr();
    bdAddr(const bdAddr& other);
    bdAddr(const bdInetAddr& address, const bdPort port);
    bdAddr(const bdNChar8* socketAddress);
    ~bdAddr();
    bdBool operator==(const bdAddr& other) const;
    bdBool operator!=(const bdAddr& other) const;

    void set(const bdNChar8* socketAddress);
    void set(const bdInetAddr& address, const bdPort port);
    void setPort(const bdPort port);

    bdUInt getHash();
    const bdInetAddr& getAddress() const;
    const bdPort getPort() const;
    bdUInt getSerializedSize();

    const bdUWord toString(bdNChar8* const str, const bdUWord size) const;
    bdBool serialize(bdUByte8* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset) const;
    bdBool deserialize(const bdUByte8* data, const bdUInt size, const bdUInt offset, bdUInt& newOffset);
};
