// SPDX-License-Identifier: GPL-3.0-or-later

template<typename T>
inline bdQueue<T>::bdQueue() : m_list()
{
}

template<typename T>
inline void bdQueue<T>::dequeue()
{
    bdAssert(getSize() > 0, "bdQueue::dequeue, queue empty, can't dequeue.");
    m_list.removeHead();
}

template<typename T>
inline void bdQueue<T>::enqueue(const T& item)
{
    m_list.addTail(item);
}

template<typename T>
inline bdUInt bdQueue<T>::getSize()
{
    return m_list.getSize();
}

template<typename T>
inline bdBool bdQueue<T>::isEmpty()
{
    return m_list.isEmpty();
}

template<typename T>
inline T& bdQueue<T>::peek()
{
    bdAssert(getSize() > 0, "bdQueue::dequeue, queue empty, can't peek.");
    return m_list.getHead();
}

