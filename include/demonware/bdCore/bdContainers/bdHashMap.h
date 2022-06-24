// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdHashingClass 
{
public:
    bdUInt getHash(const bdUByte8* key, const bdUWord size)
    {
        bdInt i;
        bdUInt hash;

        hash = 0;
        for (i = 0; i < size; ++i) {
            hash *= (0x1000193 ^ (bdInt)key[i]);
        }
        return hash;
    }
    template <typename T>
    bdUInt getHash(const T& key)
    {
        return getHash(reinterpret_cast<const bdUByte8*>(&key), sizeof(T));
    }
};

template <typename keyType, typename dataType, typename hashClass>
class bdHashMap
{
public:
    typedef void* Iterator;
    class Node
    {
    public:
        dataType m_data;
        keyType m_key;
        Node* m_next;

        void* operator new(bdUWord nbytes);
        void operator delete(void* p);
        Node();
        Node(keyType const& key, dataType const& value, Node* const next);
        ~Node();
    };
    bdUInt m_size;
    bdUInt m_capacity;
    bdFloat32 m_loadFactor;
    bdUInt m_threshold;
    Node** m_map;
    hashClass m_hashClass;
    bdUInt m_numIterators;

    bdHashMap();
    bdHashMap(const bdUInt initialCapacity, const bdFloat32 loadFactor);
    ~bdHashMap();
    Iterator getIterator()
    {
        bdUInt numIter;

        if (!m_size)
        {
            return NULL;
        }
        for (numIter = 0; numIter < m_capacity, m_map[numIter] == NULL; ++numIter);
        if (m_map[numIter])
        {
            ++m_numIterators;
        }
        return m_map[numIter];
    }
    Iterator getIterator(const keyType& key)
    {
        Node* n;
        bdUInt hash;

        if (!m_size)
        {
            return NULL;
        }
        hash = m_hashClass.getHash(key);
        for (n = m_map[getHashIndex(hash)]; n; n = n->m_next)
        {
            if (key == n->m_key)
            {
                ++m_numIterators;
                return n;
            }
        }
        return NULL;
    }
    void releaseIterator(Iterator iterator);
    dataType& getValue(Iterator iterator);
    keyType& getKey(Iterator iterator);
    void next(Iterator& iterator);
    bdBool remove(const keyType& key);
    bdBool remove(const keyType& key, dataType& value);
    bdBool remove(Iterator& iterator);
    void clear();
    void resize(const bdUInt newSize);
    bdBool get(const keyType& key, dataType& value);
    bdBool put(const keyType& key, const dataType& value);
    void createMap(const bdUInt initialCapacity, const bdFloat32 loadFactor);
    bdUInt getHashIndex(const bdUInt hash);
    bdInt getSize();
    bdBool containsKey(const keyType& key);
    static bdUInt getNextCapacity(const bdUInt targetCapacity);
};

#include "bdHashMap.inl"
