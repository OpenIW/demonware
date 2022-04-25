// SPDX-License-Identifier: GPL-3.0-or-later

template<typename keyType, typename dataType, typename hashClass>
inline bdHashMap<keyType, dataType, hashClass>::Node::Node()
{
}

template<typename keyType, typename dataType, typename hashClass>
inline bdHashMap<keyType, dataType, hashClass>::Node::Node(keyType* key, dataType* value, Node* const next) : m_data(value), m_key(*key)
{
    m_next = next;
}

template<typename keyType, typename dataType, typename hashClass>
inline bdHashMap<keyType, dataType, hashClass>::Node::~Node()
{
    delete this;
}

template<typename keyType, typename dataType, typename hashClass>
inline bdHashMap<keyType, dataType, hashClass>::bdHashMap()
{
    m_numIterators = 0;
}

template<typename keyType, typename dataType, typename hashClass>
inline bdHashMap<keyType, dataType, hashClass>::bdHashMap(const bdUInt initialCapacity, const bdFloat32 loadFactor)
{
    m_numIterators = 0;
    createMap(initialCapacity, loadFactor);
}

template<typename keyType, typename dataType, typename hashClass>
inline bdHashMap<keyType, dataType, hashClass>::~bdHashMap()
{
    clear();
    bdDeallocate<bdHashMap<keyType, dataType, hashClass>::Node*>(m_map);
}

template<typename keyType, typename dataType, typename hashClass>
inline void bdHashMap<keyType, dataType, hashClass>::releaseIterator(Iterator iterator)
{
    if (iterator)
    {
        bdAssert(m_numIterators != 0, "bdHashMap::releaseIteratorIterator count reached 0, can't release iterator");
        --m_numIterators;
    }
}

template<typename keyType, typename dataType, typename hashClass>
inline dataType* bdHashMap<keyType, dataType, hashClass>::getValue(Iterator iterator)
{
    return &reinterpret_cast<Node*>(iterator)->m_data;
}

template<typename keyType, typename dataType, typename hashClass>
inline keyType* bdHashMap<keyType, dataType, hashClass>::getKey(Iterator iterator)
{
    return &reinterpret_cast<Node*>(iterator)->m_key;
}

template<typename keyType, typename dataType, typename hashClass>
inline void bdHashMap<keyType, dataType, hashClass>::next(Iterator* iterator)
{
    bdUInt hash;
    Node* n;

    n = reinterpret_cast<Node*>(*iterator);

    if (n->m_next)
    {
        n = n->m_next;
    }
    else
    {
        hash = m_hashClass.getHash(&n->m_key);
        for (bdUInt i = getHashIndex(hash) + 1; i < m_capacity; ++i)
        {
            if (m_map[i])
            {
                n = m_map[i];
                return;
            }
        }
        n = NULL;
        --m_numIterators;
    }
}

template<typename keyType, typename dataType, typename hashClass>
inline bdBool bdHashMap<keyType, dataType, hashClass>::remove(keyType* key)
{
    Node* prevNode;
    Node* n;
    bdUInt hash;
    bdUInt i;

    bdAssert(m_numIterators == 0, "bdHashMap::remove, another iterator is being held while removing from hashmap");
    hash = m_hashClass.getHash(key);
    i = getHashIndex(hash);
    n = m_map[i];
    prevNode = NULL;
    while (1)
    {
        if (!n)
        {
            return false;
        }
        if (key == &n->m_key)
        {
            break;
        }
        prevNode = n;
        n = n->m_next;
    }
    if (prevNode)
    {
        prevNode->m_next = n->m_next;
    }
    else
    {
        m_map[i] = n->m_next;
    }
    --m_size;

    return true;
}

template<typename keyType, typename dataType, typename hashClass>
inline bdBool bdHashMap<keyType, dataType, hashClass>::remove(const keyType* key, dataType* value)
{
    Node* n;
    Node* prevNode;
    bdUInt i;
    bdUInt hash;

    hash = m_hashClass.getHash(key);
    i = getHashIndex(hash);
    n = m_map[i];
    prevNode = NULL;
    bdAssert(m_numIterators == 0, "bdHashMap::remove, another iterator is being held while removing from hashmap");
    while (1)
    {
        if (!n)
        {
            return false;
        }
        if (*const_cast<keyType*>(key) == n->m_key)
        {
            break;
        }
        prevNode = n;
        n = n->m_next;
    }
    if (prevNode)
    {
        prevNode->m_next = n->m_next;
    }
    else
    {
        m_map[i] = n->m_next;
    }
    value = &n->m_data;
    --m_size;
    return true;
}

template<typename keyType, typename dataType, typename hashClass>
inline bdBool bdHashMap<keyType, dataType, hashClass>::remove(Iterator* iterator)
{
    Iterator iter;
    Node* n;

    bdAssert(m_numIterators == 1, "bdHashMap::remove, more than one iterator held  while removing from hashmap");
    iter = *iterator;
    next(&iter);
    n = reinterpret_cast<Node*>(*iterator);
    *iterator = iter;
    bdUInt numIterators = m_numIterators;
    m_numIterators = 0;
    bdBool result = remove(&n->m_key);
    m_numIterators = numIterators;
    return result;
}

template<typename keyType, typename dataType, typename hashClass>
inline void bdHashMap<keyType, dataType, hashClass>::clear()
{
    Node* n;
    Node* last;
    bdUInt i;

    bdAssert(m_numIterators == 0, "bdHashMap::clear, another iterator is being held while clearing the hashmap");

    for (i = 0; i < m_capacity; ++i)
    {
        n = m_map[i];
        while (n)
        {
            last = n;
            n = n->m_next;
            last->~Node();
            delete last;
        }
        m_map[i] = NULL;
    }
    m_size = 0;
}

template<typename keyType, typename dataType, typename hashClass>
inline void bdHashMap<keyType, dataType, hashClass>::resize(const bdUInt newSize)
{
    bdUInt i;
    bdUInt targetCapacity;
    bdUInt oldCapacity;
    Node* prev;
    Node* n;
    Node** oldmap;

    oldmap = m_map;
    oldCapacity = m_capacity;
    targetCapacity = getNextCapacity(newSize);
    if (targetCapacity <= m_capacity)
    {
        return;
    }
    m_capacity = targetCapacity;
    m_threshold = (static_cast<bdFloat32>(targetCapacity) * m_loadFactor);
    m_map = bdAllocate<Node*>(m_capacity);
    m_size = 0;
    bdMemset(m_map, 0, 8 * m_capacity);
    for (i = 0; i < oldCapacity; ++i)
    {
        n = oldmap[i];
        while (n)
        {
            put(&n->m_key, &n->m_data);
            prev = n;
            n = n->m_next;

            prev->~Node();
            delete prev;
        }
    }
    bdDeallocate<Node*>(oldmap);
}

template<typename keyType, typename dataType, typename hashClass>
inline bdBool bdHashMap<keyType, dataType, hashClass>::get(const keyType* key, const dataType* value)
{
    Iterator iterator = getIterator(key);
    if (!iterator)
    {
        return false;
    }
    value = getValue(iterator);
    releaseIterator(iterator);
    return true;
}

template<typename keyType, typename dataType, typename hashClass>
inline bdBool bdHashMap<keyType, dataType, hashClass>::put(const keyType* key, const dataType* value)
{
    Node* n;
    bdUInt i;
    bdUInt hash;

    bdAssert(m_numIterators == 0, "bdHashMap::put, another iterator is being held while inserting to hashmap");

    hash = m_hashClass.getHash(key);
    i = getHashIndex(hash);
    for (n = m_map[i]; n; n = n->m_next)
    {
        if (const_cast<keyType*>(key) == &n->m_key)
        {
            return false;
        }
    }
    if (m_size + 1 > m_threshold)
    {
        resize(2 * m_capacity);
        i = getHashIndex(hash);
    }
    ++m_size;
    m_map[i] = new Node(const_cast<keyType*>(key), const_cast<dataType*>(value), m_map[i]);
    return true;
}

template<typename keyType, typename dataType, typename hashClass>
inline void bdHashMap<keyType, dataType, hashClass>::createMap(const bdUInt initialCapacity, const bdFloat32 loadFactor)
{
    if (loadFactor <= 0.0 || loadFactor > 1.0)
    {
        bdLogWarn("hashmap", "Illegal loadFactor. Using default value.");
    }
}

template<typename keyType, typename dataType, typename hashClass>
inline bdUInt bdHashMap<keyType, dataType, hashClass>::getHashIndex(const bdUInt hash)
{
    return hash & m_capacity - 1;
}

template<typename keyType, typename dataType, typename hashClass>
inline bdInt bdHashMap<keyType, dataType, hashClass>::getSize()
{
    return this->m_size;
}

template<typename keyType, typename dataType, typename hashClass>
inline bdBool bdHashMap<keyType, dataType, hashClass>::containsKey(const keyType* key)
{
    Iterator iterator = getIterator(key);
    releaseIterator(iterator);
    return iterator != NULL;
}

template<typename keyType, typename dataType, typename hashClass>
inline bdUInt bdHashMap<keyType, dataType, hashClass>::getNextCapacity(const bdUInt targetCapacity)
{
    return bdBitOperations::nextPowerOf2(targetCapacity);
}
