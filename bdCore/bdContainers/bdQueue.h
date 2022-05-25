// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

template <typename T>
class bdQueue
{
protected:
    bdLinkedList<T> m_list;
public:
    bdQueue();
    void dequeue();
    void enqueue(const T& item);
    bdUInt getSize();
    bdBool isEmpty();
    T& peek();
};

#include "bdQueue.inl"
