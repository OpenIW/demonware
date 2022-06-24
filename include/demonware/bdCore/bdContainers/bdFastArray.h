// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

template <typename T>
class bdFastArray
{
public:
    T* m_data;
    bdUInt m_capacity;
    bdUInt m_size;

    ~bdFastArray();
    bdFastArray();
    bdFastArray(const bdUInt capacity);
    bdFastArray(const bdUInt capacity, const T& value);
    bdFastArray(const bdFastArray<T>& a);
    T* uninitializedCopy(const bdFastArray<T>& a);
    void copyArrayArray(T* dest, const T* src, const bdUInt n);
    void copyArrayObject(T* dest, const T& src, const bdUInt n);
    void copyObjectObject(T* dest, const T& src);
    void ensureCapacity(const bdUInt capacity);
    void increaseCapacity(const bdUInt increase);
    void decreaseCapacity(const bdUInt decrease);
    void pushBack(T* value, bdUInt n);
    void pushBack(const T& value);
    void removeSection(const bdUInt begin, const bdUInt end);
    void removeAt(const bdUInt position);
    void removeAllKeepOrder(T value);
    bdBool findFirst(const T& value, bdUInt& i);
    void clear();
    bdBool rangeCheck(const bdUInt i);
    void setGrow(const bdUInt i, const T& value);
    T* begin();
    T* end();
    T& operator[](const bdUInt i);
    bdUInt getSize() const;
    bdUInt getCapacity() const;
    const T* getData() const;
};

#include "bdFastArray.inl"
