// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

template<typename T>
class bdArray
{
protected:
    T* m_data;
    bdUInt m_capacity;
    bdUInt m_size;
public:
    bdArray();
    bdArray(const bdUInt capacity);
    bdArray(const bdArray<T>* a);
    bdUInt getCapacity();
    bdUInt getSize();
    T* uninitializedCopy(const bdArray<T>* a);
    void clear();
    void pushBack(const T* value);
    void copyConstructObjectObject(T* dest, const T* src);
    void copyConstructArrayArray(T* dest, const T* src, unsigned int n);
    void destruct(T* src, bdUInt n);
    void increaseCapacity(unsigned int increase);
    void decreaseCapacity(const bdUInt decrease);
    ~bdArray();
    bdBool rangeCheck(const bdUInt i);
    bdBool isEmpty();
    T* operator[](const bdUInt i);
    void operator=(bdArray<T>* a);
};

#include "bdArray.inl"
