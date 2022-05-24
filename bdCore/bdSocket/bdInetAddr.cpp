// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdCore/bdCore.h"

void* bdInetAddr::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

void bdInetAddr::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdInetAddr::operator new(bdUWord nbytes, void* p)
{
    return p;
}

bdBool bdInetAddr::operator==(const bdInetAddr* other)
{
    return m_addr.inUn.m_iaddr == other->m_addr.inUn.m_iaddr;
}

bdInetAddr bdInetAddr::Broadcast()
{
    bdInetAddr addr;

    addr.set(0xFFFFFFFF);
    bdInetAddr* InetAddr(&addr);
    return InetAddr;
}

bdInetAddr bdInetAddr::All()
{
    bdInetAddr addr;

    addr.set(0u);
    bdInetAddr* InetAddr(&addr);
    return InetAddr;
}

bdInetAddr bdInetAddr::Any()
{
    bdInetAddr retstr;

    retstr.set((bdUInt)0);
    return retstr;
}

bdInetAddr::bdInetAddr()
{
}

bdInetAddr::~bdInetAddr()
{
    m_addr.inUn.m_iaddr = 0xDEADBEEF;
}

bdInetAddr::bdInetAddr(const bdInetAddr* other)
{
    *this = *other;
}

bdInetAddr::bdInetAddr(const char* address)
{
    set(address);
}

bdInetAddr::bdInetAddr(unsigned int address)
{
    set(address);
}

bdInetAddr::bdInetAddr(bdInAddr address)
{
    m_addr = address;
}

void bdInetAddr::set(const bdInetAddr* other)
{
    *this = *other;
}

void bdInetAddr::set(const char* address)
{
    m_addr.fromString(address);
}

void bdInetAddr::set(unsigned int address)
{
    m_addr.inUn.m_iaddr = address;
}

const bdBool bdInetAddr::isValid() const
{
    bdInetAddr* chk = &bdInetAddr();

    return (this == chk) == 0;
}

const bdUInt bdInetAddr::toString(char* str, unsigned int size) const
{
    if (isValid())
    {
        return m_addr.toString(str, size);
    }
    return 0;
}

const bdInAddr bdInetAddr::getInAddr() const
{
    return m_addr;
}

bool bdInetAddr::serialize(bdUByte8* data, unsigned int size, unsigned int offset, unsigned int* newOffset)
{
    return bdBytePacker::appendBuffer(data, size, offset, newOffset, &m_addr, sizeof(m_addr));
}

bool bdInetAddr::deserialize(const bdUByte8* data, unsigned int size, unsigned int offset, unsigned int* newOffset)
{
    return bdBytePacker::removeBuffer(data, size, offset, newOffset, &m_addr, sizeof(m_addr));
}

