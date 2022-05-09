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
    bdArray(const bdUInt capacity, T* value);
    const bdUInt getCapacity() const;
    const bdUInt getSize() const;
    bdBool get(const bdUInt i, T* value);
    bdBool findFirst(const T* value, bdUInt* i) const;
    T* uninitializedCopy(const bdArray<T>* a);
    void clear();
    void pushBack(const T* value);
    void pushBack(const T* value, const bdUInt n);
    void popBack();
    void copyConstructObjectObject(T* dest, const T* src);
    void copyConstructArrayArray(T* dest, const T* src, unsigned int n);
    void copyConstructArrayObject(T* dest, const T* src, bdUInt n);
    void destruct(T* src, bdUInt n);
    void increaseCapacity(unsigned int increase);
    void decreaseCapacity(const bdUInt decrease);
    void removeAt(const bdUInt i);
    ~bdArray();
    bdBool rangeCheck(const bdUInt i);
    bdBool isEmpty();
    const T* operator[](const bdUInt i) const;
    T* operator[](const bdUInt i);
    void operator=(bdArray<T>* a);
};

#include "bdArray.inl"
