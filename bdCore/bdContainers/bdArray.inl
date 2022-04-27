#include "bdArray.h"
// SPDX-License-Identifier: GPL-3.0-or-later

template<typename T>
inline bdArray<T>::bdArray()
{
}

template<typename T>
inline bdArray<T>::bdArray(const bdUInt capacity)
{
    m_data = NULL;
    m_capacity = capacity;
    if (m_capacity)
    {
        m_data = bdAllocate<T>(m_capacity);
    }
}

template<typename T>
inline bdArray<T>::bdArray(const bdArray<T>* a)
{
    m_capacity = a->m_capacity;
    m_size = a->m_size;
    m_data = uninitializedCopy(a);
}

template<typename T>
inline bdArray<T>::bdArray(const bdUInt capacity, T* value)
{
    m_data = NULL;
    m_capacity = capacity;
    m_size = capacity;
    if (m_capacity)
    {
        m_data = bdAllocate<T>(m_capacity);
        copyConstructArrayObject(m_data, value, m_capacity);
    }
}

template<typename T>
inline bdUInt bdArray<T>::getCapacity()
{
    return m_capacity;
}

template<typename T>
inline bdUInt bdArray<T>::getSize()
{
    return m_size;
}

template<typename T>
T* bdArray<T>::uninitializedCopy(const bdArray<T>* a)
{
    T* data = NULL;
    if (a->m_capacity)
    {
        data = bdAllocate<T>(a->m_capacity);
        copyConstructArrayArray(data, a->m_data, a->m_size);
    }
    return data;
}

template<typename T>
void bdArray<T>::clear()
{
    destruct(m_data, m_size);
    bdDeallocate<T>(m_data);
    m_data = NULL;
    m_size = 0;
    m_capacity = 0;
}

template<typename T>
void bdArray<T>::pushBack(const T* value)
{
    if (this->m_size == this->m_capacity)
    {
        increaseCapacity(1);
    }
    if (&this->m_data[this->m_size])
    {
        copyConstructObjectObject(&this->m_data[this->m_size], value);
    }
    ++this->m_size;
}

template<typename T>
inline void bdArray<T>::pushBack(const T* value, const bdUInt n)
{
    bdUInt spare = this->m_capacity - this->m_size;
    if (n > spare)
    {
        increaseCapacity(n - spare);
    }
    copyConstructArrayArray(&this->m_data[this->m_size], value, n);
}

template<typename T>
inline void bdArray<T>::popBack()
{
    if (m_size)
    {
        destruct(&m_data[--m_size], 1u);
        decreaseCapacity(0);
    }
}

template<typename T>
void bdArray<T>::copyConstructObjectObject(T* dest, const T* src)
{
    dest = new T(src);
}

template<typename T>
void bdArray<T>::copyConstructArrayArray(T* dest, const T* src, unsigned int n)
{
    for (int i = 0; i < n; ++i)
    {
        dest[i] = new T(src[i]);
    }
}

template<typename T>
inline void bdArray<T>::copyConstructArrayObject(T* dest, const T* src, bdUInt n)
{
    for (bdUInt i = 0; i < n; ++i)
    {
        dest[i] = new T(*src);
    }
}

template<typename T>
void bdArray<T>::destruct(T* src, bdUInt n)
{
    for (bdUInt i = 0; i < n; ++i)
    {
        delete &src[i];
    }
}

template<typename T>
void bdArray<T>::increaseCapacity(unsigned int increase)
{
    T* newData;
    bdUInt newCapacity;

    newCapacity = (increase <= this->m_capacity ? m_capacity : increase) + this->m_capacity;
    newData = NULL;
    if (newCapacity)
    {
        newData = bdAllocate<T>(newCapacity);
        copyConstructArrayArray(newData, this->m_data, this->m_size);
    }
    destruct(this->m_data, this->m_size);
    bdDeallocate<T>(this->m_data);
    this->m_data = newData;
    this->m_capacity = newCapacity;
}

template<typename T>
void bdArray<T>::decreaseCapacity(const bdUInt decrease)
{
    bdUInt newCapacity;
    T* newData;

    if (this->m_capacity > sizeof(T) * this->m_size)
    {
        if (decrease <= this->m_capacity - this->m_size)
        {
            newCapacity = decrease;
        }
        else
        {
            newCapacity = this->m_capacity - this->m_size;
        }

        if (newCapacity <= this->m_capacity >> 1)
        {
            newCapacity = this->m_capacity >> 1;
        }
        this->m_capacity -= newCapacity;
        newData = NULL;
        if (this->m_capacity)
        {
            newData = bdAllocate<T>(this->m_capacity);
            copyConstructArrayArray(newData, this->m_data, this->m_size);
        }
        destruct(this->m_data, this->m_size);
        bdDeallocate<T>(this->m_data);
        this->m_data = newData;
    }
}

template<typename T>
inline void bdArray<T>::removeAt(const bdUInt i)
{
    if (rangeCheck(i))
    {
        m_data[i] = m_data[m_size - 1];
        popBack();
    }
}

template<typename T>
bdArray<T>::~bdArray()
{
    clear();
}

template<typename T>
bdBool bdArray<T>::rangeCheck(const bdUInt i)
{
    return i < this->m_size;
}

template<typename T>
inline bdBool bdArray<T>::isEmpty()
{
    return m_size == 0;
}

template<typename T>
T* bdArray<T>::operator[](const bdUInt i)
{
    return &this->m_data[i];
}

template<typename T>
void bdArray<T>::operator=(bdArray<T>* a)
{
    bdUInt newSize;

    if (this == a)
    {
        return;
    }
    newSize = a->getSize();
    if (newSize > this->m_capacity)
    {
        clear();
        this->m_data = uninitializedCopy(a);
        this->m_capacity = a->m_capacity;
        this->m_size = newSize;
        return;
    }

    if (newSize <= this->m_size)
    {
        for (bdUInt i = 0; i < newSize; ++i)
        {
            &this->m_data[i] = a[i];
        }
        destruct(&this->m_data[newSize], this->m_size - newSize);
        this->m_size = newSize;
        decreaseCapacity(0);
    }
    else
    {
        for (bdUInt i = 0; i < this->m_size; ++i)
        {
            &this->m_data[i] = a[i];
        }
        copyConstructArrayArray(&this->m_data[this->m_size], &a->m_data[this->m_size], newSize - this->m_size);
        this->m_size = newSize;
    }
}