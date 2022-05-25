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
    m_size = capacity;
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
inline const bdUInt bdArray<T>::getCapacity() const
{
    return m_capacity;
}

template<typename T>
inline const bdUInt bdArray<T>::getSize() const
{
    return m_size;
}

template<typename T>
inline bdBool bdArray<T>::get(const bdUInt i, T& value)
{
    bdBool inRange = rangeCheck(i);
    if (inRange)
    {
        bdMemcpy(value, m_data[i], sizeof(T));
    }
    return inRange;
}

template<typename T>
inline bdBool bdArray<T>::findFirst(const T* value, bdUInt* i) const
{
    for (bdUInt j = 0; j < m_size; ++j)
    {
        if (value == &m_data[j])
        {
            *i = j;
            return true;
        }
    }
    return false;
}

template<typename T>
T* bdArray<T>::uninitializedCopy(const bdArray<T>& a)
{
    T* data = NULL;
    if (a.getCapacity())
    {
        data = bdAllocate<T>(a.getCapacity());
        copyConstructArrayArray(data, a.m_data, a.getSize());
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
void bdArray<T>::pushBack(const T& value)
{
    if (m_size == m_capacity)
    {
        increaseCapacity(1);
    }
    copyConstructObjectObject(&m_data[m_size], value);
    ++m_size;
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
void bdArray<T>::copyConstructObjectObject(T* dest, const T& src)
{
    dest = new(dest) T(src);
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
inline void bdArray<T>::copyConstructArrayObject(T* dest, const T& src, bdUInt n)
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
        src[i].~T();
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
inline const T& bdArray<T>::operator[](const bdUInt i) const
{
    return m_data[i];
}

template<typename T>
inline T& bdArray<T>::operator[](const bdUInt i)
{
    return m_data[i];
}

template<typename T>
void bdArray<T>::operator=(const bdArray<T>& a)
{
    bdUInt newSize;

    if (this == a)
    {
        return;
    }
    newSize = a.getSize();
    if (newSize > m_capacity)
    {
        clear();
        m_data = uninitializedCopy(a);
        m_capacity = a.getCapacity();
        m_size = newSize;
        return;
    }

    if (newSize <= m_size)
    {
        for (bdUInt i = 0; i < newSize; ++i)
        {
            &m_data[i] = a[i];
        }
        destruct(&m_data[newSize], m_size - newSize);
        m_size = newSize;
        decreaseCapacity(0);
    }
    else
    {
        for (bdUInt i = 0; i < m_size; ++i)
        {
            &m_data[i] = a[i];
        }
        copyConstructArrayArray(&m_data[m_size], &a.m_data[m_size], newSize - m_size);
        m_size = newSize;
    }
}
