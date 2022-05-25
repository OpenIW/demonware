// SPDX-License-Identifier: GPL-3.0-or-later

inline bdReferencable::~bdReferencable()
{
    delete this;
}

inline bdReferencable& bdReferencable::operator=(bdReferencable& a)
{
    return *this;
}

inline bdInt bdReferencable::releaseRef()
{
    return InterlockedDecrement((volatile LONG*)&m_refCount) - 1;
}

inline bdInt bdReferencable::addRef()
{
    return InterlockedIncrement((volatile LONG*)&m_refCount) + 1;
}

inline bdInt bdReferencable::getRefCount()
{
    return m_refCount;
}

inline bdReferencable::bdReferencable()
{
    m_refCount = 0;
}

inline void bdReferencable::operator delete(void* p)
{
    bdMemory::deallocate(p);
}
