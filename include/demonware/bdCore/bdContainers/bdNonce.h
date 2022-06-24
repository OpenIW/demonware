// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

template <bdInt SIZE>
class bdNonce
{
public:
    bdNChar8 m_nonce[SIZE];
    bool m_initialised;

    bdNonce();
    void ensureCreated();
    const bdUByte8* getData();
};

#include "bdNonce.inl"