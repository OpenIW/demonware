// SPDX-License-Identifier: GPL-3.0-or-later

template<typename T>
inline bdFastArray<T>::~bdFastArray()
{
    clear();
}

template<typename T>
inline bdFastArray<T>::bdFastArray()
    : m_data(NULL), m_capacity(0), m_size(0)
{
    if (m_capacity)
    {
        m_data = bdAllocate<T>(m_capacity);
    }
}

template<typename T>
inline bdFastArray<T>::bdFastArray(const bdUInt capacity)
    : m_data(NULL), m_capacity(capacity), m_size(capacity)
{
    if (m_capacity)
    {
        m_data = bdAllocate<T>(m_capacity);
    }
}

template<typename T>
inline bdFastArray<T>::bdFastArray(const bdUInt capacity, const T& value)
    : m_data(NULL), m_capacity(capacity), m_size(capacity)
{
    if (m_capacity)
    {
        m_data = bdAllocate<T>(m_capacity);
        copyArrayObject(m_data, value, m_capacity);
    }
}

template<typename T>
inline bdFastArray<T>::bdFastArray(const bdFastArray<T>& a)
    : m_capacity(a.getCapacity()), m_size(a.getSize()), m_data(uninitializedCopy(a))
{
}

template<typename T>
inline T* bdFastArray<T>::uninitializedCopy(const bdFastArray<T>& a)
{
    T* data = NULL;
    if (a->getCapacity())
    {
        data = bdAllocate<T>(a.getCapacity());
        copyArrayArray(data, a.getData(), a.getSize());
    }
    return data;
}

template<typename T>
inline void bdFastArray<T>::copyArrayArray(T* dest, const T* src, const bdUInt n)
{
    if (n)
    {
        bdMemcpy(dest, src, sizeof(T) * n);
    }
}

template<typename T>
inline void bdFastArray<T>::copyArrayObject(T* dest, const T& src, const bdUInt n)
{
    for (bdUInt i = 0; i < n; ++i)
    {
        bdMemcpy(&dest[i], src, sizeof(T));
    }
}

template<typename T>
inline void bdFastArray<T>::copyObjectObject(T* dest, const T& src)
{
    bdMemcpy(dest, src, sizeof(T));
}

template<typename T>
inline void bdFastArray<T>::ensureCapacity(const bdUInt capacity)
{
    if (m_capacity < capacity)
    {
        increaseCapacity(capacity - m_capacity);
    }
}

template<typename T>
inline void bdFastArray<T>::increaseCapacity(const bdUInt increase)
{
    bdUInt newCapacity = (increase <= m_capacity ? m_capacity : increase) + m_capacity;
    T* newData = NULL;
    if (newCapacity)
    {
        newData = bdAllocate<T>(newCapacity);
        copyArrayArray(newData, m_data, m_size);
    }
    bdDeallocate<T>(m_data);
    m_data = newData;
    m_capacity = newCapacity;
}

template<typename T>
inline void bdFastArray<T>::decreaseCapacity(const bdUInt decrease)
{
    bdUInt newCapacity;

    if (m_capacity > 4 * m_size)
    {
        if (decrease <= m_capacity - m_size)
        {
            newCapacity = decrease;
        }
        else
        {
            newCapacity = m_capacity - m_size;
        }
        if (newCapacity <= m_capacity >> 1)
        {
            newCapacity = m_capacity >> 1;
        }
        m_capacity -= newCapacity;
        T* newData = 0LL;
        if (m_capacity)
        {
            newData = bdReallocate<T>(m_data, m_capacity);
        }
        m_data = newData;
    }
}

template<typename T>
inline void bdFastArray<T>::pushBack(T* value, bdUInt n)
{
    ensureCapacity(n + m_size);
    copyArrayArray(&m_data[m_size], value, n);
    m_size += n;
}

template<typename T>
inline void bdFastArray<T>::pushBack(const T& value)
{
    if (m_size == m_capacity)
    {
        increaseCapacity(1);
    }
    copyObjectObject(&m_data[m_size], value);
    ++m_size;
}

template<typename T>
inline void bdFastArray<T>::removeSection(const bdUInt begin, const bdUInt end)
{
    bdBool inRange = false;
    if (rangeCheck(begin) && end <= m_size)
    {
        inRange = begin < end;
    }
    if (inRange)
    {
        bdMemmove(&m_data[begin], &m_data[end], sizeof(T) * (m_size - end));
        m_size -= end - begin;
        decreaseCapacity(0);
    }
}

template<typename T>
inline void bdFastArray<T>::removeAt(const bdUInt position)
{
    if (position < m_size)
    {
        m_data[position] = m_data[m_size - 1];
        if (m_size)
        {
            --m_size;
            decreaseCapacity(0);
        }
    }
}

template<typename T>
inline void bdFastArray<T>::removeAllKeepOrder(T value)
{
    for (bdUInt i = 0; i < m_size; ++i)
    {
        if (value == m_data[i])
        {
            removeSection(i, i + 1);
            --i;
        }
    }
}

template<typename T>
inline bdBool bdFastArray<T>::findFirst(const T& value, bdUInt& i)
{
    for (bdUInt j = 0; j < m_size; ++j)
    {
        if (value == m_data[j])
        {
            i = j;
            return true;
        }
    }
    return false;
}

template<typename T>
inline void bdFastArray<T>::clear()
{
    bdDeallocate<T>(m_data);
    m_data = NULL;
    m_size = 0;
    m_capacity = 0;
}

template<typename T>
inline bdBool bdFastArray<T>::rangeCheck(const bdUInt i)
{
    return i < m_size;
}

template<typename T>
inline void bdFastArray<T>::setGrow(const bdUInt i, const T& value)
{
    if (!rangeCheck(i))
    {
        ensureCapacity(i + 1);
        m_size = i + 1;
    }
    m_data[i] = value;
}

template<typename T>
inline T* bdFastArray<T>::begin()
{
    return m_data;
}

template<typename T>
inline T* bdFastArray<T>::end()
{
    return m_data + m_size;
}

template<typename T>
inline T& bdFastArray<T>::operator[](const bdUInt i)
{
    bdAssert(rangeCheck(i), "bdFastArray<T>::operator[], rangecheck failed");
    return m_data[i];
}

template<typename T>
inline bdUInt bdFastArray<T>::getSize() const
{
    return m_size;
}

template<typename T>
inline bdUInt bdFastArray<T>::getCapacity() const
{
    return m_capacity;
}

template<typename T>
inline const T* bdFastArray<T>::getData() const
{
    return m_data;
}
