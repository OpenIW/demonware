// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdLinkable
{
public:
    bdLinkable* m_previous;
    bdLinkable* m_next;
    bdLinkable** m_headPtr;

    bdLinkable();
    virtual ~bdLinkable();
    void link(bdLinkable** head);
    void unlink();
    bdLinkable* getNext();
    static bdBool find(bdLinkable* list, bdLinkable* link);
};
