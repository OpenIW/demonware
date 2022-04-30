// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

template <typename T>
class bdCreatorBase
{
public:
    void operator delete(void* p);
    virtual ~bdCreatorBase();
    virtual T* create();
    virtual bdUInt getSizeOf();
};

#include "bdFactory.inl"