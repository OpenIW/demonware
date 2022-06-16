// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

typedef void* Position;

template<typename T>
class bdLinkedList
{
public:
    typedef void* Position;
    class Node
    {
    public:
        T m_data;
        Node* m_next;
        Node* m_prev;

        void operator delete(void* p)
        {
            bdMemory::deallocate(p);
        }
        void* operator new(bdUWord nBytes)
        {
            return bdMemory::allocate(nBytes);
        }
        Node(const T& data) : m_data(data)
        {
        }
        ~Node()
        {

        }
    };
    Node* m_head;
    Node* m_tail;
    bdUInt m_size;

    bdLinkedList();
    ~bdLinkedList();

    void clear();
    T& forward(Position& position) const;
    void addTail(const T& value);
    T& getHead() const;
    Position getHeadPosition() const;
    bdUInt getSize() const;
    T& getAt(const Position position) const;
    Position getTailPosition() const;
    void insertAfter(const Position position, const T& value);
    const bdBool isEmpty() const;
    void removeAt(Position& position);
    void removeHead();
};

#include "bdLinkedList.inl"
