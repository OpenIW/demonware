// SPDX-License-Identifier: GPL-3.0-or-later

template<typename T>
inline bdLinkedList<T>::bdLinkedList()
{
    m_head = NULL;
    m_tail = NULL;
    m_size = 0;
}

template<typename T>
inline bdLinkedList<T>::~bdLinkedList()
{
    clear();
}

template<typename T>
inline void bdLinkedList<T>::clear()
{
    Node* p;
    Node* next;

    while (m_head)
    {
        next = m_head->m_next;
        p = m_head;
        if (m_head)
        {
            delete p;
        }
        m_head = next;
    }
    m_tail = NULL;
    m_size = 0;
}

template<typename T>
inline T& bdLinkedList<T>::forward(Position& position) const
{
    position = reinterpret_cast<Node*>(position)->m_next;
    return reinterpret_cast<Node*>(position)->m_data;
}

template<typename T>
inline void bdLinkedList<T>::addTail(const T& value)
{
    Position position;

    position = getTailPosition();
    insertAfter(position, value);
}

template<typename T>
inline T& bdLinkedList<T>::getHead() const
{
    return m_head->m_data;
}

template<typename T>
inline Position bdLinkedList<T>::getHeadPosition() const
{
    return m_head;
}

template<typename T>
inline bdUInt bdLinkedList<T>::getSize() const
{
    return m_size;
}

template<typename T>
inline T& bdLinkedList<T>::getAt(const Position position) const
{
    return *reinterpret_cast<T*>(position);
}

template<typename T>
inline Position bdLinkedList<T>::getTailPosition() const
{
    return m_tail;
}

template<typename T>
inline void bdLinkedList<T>::insertAfter(const Position position, const T& value)
{
    Node* node = new Node(value);

    if (position)
    {
        node->m_next = reinterpret_cast<Node*>(position)->m_next;
        node->m_prev = reinterpret_cast<Node*>(position);

        if (reinterpret_cast<Node*>(position)->m_next)
        {
            reinterpret_cast<Node*>(position)->m_next->m_prev = node;
        }
        else
        {
            m_tail = node;
        }
        reinterpret_cast<Node*>(position)->m_next = node;
    }
    else
    {
        node->m_next = NULL;
        node->m_prev = m_tail;
        if (m_tail)
        {
            m_tail->m_next = node;
        }
        else
        {
            m_head = node;
        }
    }
    ++m_size;
}

template<typename T>
inline const bdBool bdLinkedList<T>::isEmpty() const
{
    return m_size == 0;
}

template<typename T>
inline void bdLinkedList<T>::removeAt(Position& position)
{
    Node* node;

    node = reinterpret_cast<Node*>(position);
    if (position)
    {
        forward(position);
        if (node == m_head)
        {
            m_head = m_head->m_next;
        }
        else
        {
            node->m_prev->m_next = node->m_next;
        }

        if (node == m_tail)
        {
            m_tail = node->m_prev;
        }
        else
        {
            node->m_next = node->m_prev;
        }

        if (node)
        {
            node->~Node();
        }
        --m_size;
    }
}

template<typename T>
inline void bdLinkedList<T>::removeHead()
{
    Position headPosition;

    headPosition = getHeadPosition();
    removeAt(headPosition);
}
