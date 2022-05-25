// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdInetAddr
{
protected:
    bdInAddr m_addr;
public:
    void* operator new(const bdUWord nbytes);
    void operator delete(void* p);
    void* operator new(const bdUWord nbytes, void* p);
    bdBool operator==(const bdInetAddr& other) const;
    static bdInetAddr Broadcast();
    static bdInetAddr All();
    static bdInetAddr Any();
    bdInetAddr();
    ~bdInetAddr();
    bdInetAddr(const bdInetAddr& other);
    bdInetAddr(const char* address);
    bdInetAddr(unsigned int address);
    bdInetAddr(bdInAddr address);
    void set(const bdInetAddr& other);
    void set(const char* address);
    void set(unsigned int address);
    const bdBool isValid() const;
    const bdUInt toString(char* str, bdUInt size) const;
    const bdInAddr getInAddr() const;
    bool serialize(bdUByte8* data, bdUInt size, bdUInt offset, bdUInt& newOffset) const;
    bool deserialize(const bdUByte8* data, bdUInt size, bdUInt offset, bdUInt& newOffset);
};
