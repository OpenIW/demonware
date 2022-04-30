// SPDX-License-Identifier: GPL-3.0-or-later

template<typename T>
inline void bdCreatorBase<T>::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

template<typename T>
inline bdCreatorBase<T>::~bdCreatorBase()
{
}

template<typename T>
inline T* bdCreatorBase<T>::create()
{
    return NULL;
}

template<typename T>
inline bdUInt bdCreatorBase<T>::getSizeOf()
{
    return 0;
}