// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

template <typename T>
class bdFastArray
{
public:
    T* m_data;
    unsigned int m_capacity;
    unsigned int m_size;

    bdFastArray()
    {
        m_data = NULL;
        m_capacity = 0;
        m_size = 0;
    };
    bdFastArray(const bdUInt capacity) : m_data(NULL), m_capacity(capacity)
    {
        if (m_capacity)
        {
            m_data = bdAllocate<T>(m_capacity);
        }
    };
    void ensureCapacity(unsigned int capacity)
    {
        unsigned int newCapacity;
        unsigned int ensuredCapacity;
        void* newData = 0;

        if (m_capacity < capacity)
        {
            ensuredCapacity = capacity - m_capacity;
            if (ensuredCapacity <= m_capacity)
            {
                ensuredCapacity = m_capacity;
            }

            newCapacity = m_capacity + ensuredCapacity;
            if (newCapacity)
            {
                newData = bdMemory::allocate(newCapacity * sizeof(T));
                if (m_size)
                {
                    memcpy(newData, m_data, m_size * sizeof(T));
                }
            }
            bdMemory::deallocate(m_data);
            m_data = reinterpret_cast<T*>(newData);
            m_capacity = newCapacity;
        }
    };

    void increaseCapacity(unsigned int increase)
    {
        void* newAlloc = 0;

        if (increase <= m_capacity)
        {
            increase = m_capacity;
        }

        unsigned int newSize = increase + m_capacity;
        if (newSize)
        {
            newAlloc = bdMemory::allocate(sizeof(T) * newSize);
            if (m_size)
            {
                memcpy(newAlloc, m_data, sizeof(T) * m_size);
            }
        }
        bdMemory::deallocate(m_data);
        m_capacity = newSize;
        m_data = newAlloc;
    };

    void decreaseCapacity(unsigned int decrease)
    {
        unsigned int newCapacity;
        T* newData = 0;

        if (m_capacity > sizeof(T) * m_size)
        {
            if (decrease <= m_capacity - m_size)
            {
                newCapacity = m_size;
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

            if (m_capacity)
            {
                newData = bdReallocate<T>(m_data, m_capacity);
            }
            m_data = newData;
        }
    };

    void pushBack(T* value, unsigned int n)
    {
        unsigned int totalSize = n + m_size;

        if (m_capacity < totalSize)
        {
            totalSize -= m_capacity;
            if (totalSize <= m_capacity)
            {
                totalSize = m_capacity;
            }

            unsigned int newCapacity = m_capacity + totalSize;
            void* newData = 0;
            if (newCapacity)
            {
                newData = bdMemory::allocate(newCapacity * sizeof(T));
                if (m_size)
                {
                    memcpy(newData, m_data, m_size * sizeof(T));
                }
            }

            bdMemory::deallocate(m_data);
            m_data = reinterpret_cast<T*>(newData);
            m_capacity = newCapacity;
        }
        if (n)
        {
            memcpy(&m_data[m_size], value, n);
        }
        m_size += n;
    };

    void pushBack(T const* value)
    {
        unsigned int totalSize = sizeof(T);
        unsigned int newCapacity;
        T* newData = 0;

        if (m_size == m_capacity)
        {
            if (m_capacity)
            {
                totalSize = m_capacity;
            }

            newCapacity = totalSize + m_capacity;
            if (newCapacity)
            {
                newData = bdAllocate<T>(newCapacity);
                if (m_size)
                {
                    bdMemcpy(newData, m_data, newCapacity * sizeof(T));
                }
            }
            bdDeallocate<T>(m_data);
            m_data = newData;
            m_capacity = newCapacity;
        }
        bdMemcpy(m_data[m_size], value, sizeof(m_data[m_size]));
        ++m_size;
    };

    void removeSection(unsigned int v1, unsigned int v2)
    {
        if (v1 < m_size && v2 <= m_size)
        {
            if (v1 < v2)
            {
                memmove(m_data[v1], m_data[v2], sizeof(T) * (m_size - v2));
                m_size -= v2 - v2;
                decreaseCapacity(0);
            }
        }
    };

    void removeAt(unsigned int position)
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

    unsigned int removeAllKeepOrder(T value)
    {
        unsigned int i, j;

        for (i = 0; i < m_size; ++i)
        {
            if (value == m_data[i])
            {
                removeSection(i, i + 1);
                --i;
            }
            j = i + 1;
        }
        return j;
    };

    bdBool findFirst(T const* value, bdUInt* i)
    {
        bdUInt j;

        for (j = 0; j < this->m_size; ++j)
        {
            if (*value == this->m_data[j])
            {
                *i = j;
                return true;
            }
        }
        return false;
    }

    void clear()
    {
        bdDeallocate<T>(m_data);
        m_data = NULL;
        m_size = 0;
        m_capacity = 0;
    }

    bdBool rangeCheck(const bdUInt i)
    {
        return (i < this->m_size);
    };

    void setGrow(const bdUInt i, const bdUByte8* value)
    {
        if (!rangeCheck(i))
        {
            ensureCapacity(i + 1);
            this->m_size = i + 1;
        }
        this->m_data[i] = *value;
    };

    bdUByte8* begin()
    {
        return this->m_data;
    };

    bdUInt getSize()
    {
        return this->m_size;
    };

    T* operator[](const bdUInt i)
    {
        bdAssert(rangeCheck(i), "bdFastArray<T>::operator[], rangecheck failed");
        return &m_data[i];
    }
};