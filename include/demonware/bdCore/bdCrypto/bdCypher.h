// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#define BD_CYPHER_BLOCK_SIZE_MAX 16

class bdCypher
{
protected:
    bdUInt32 m_blockSize;
public:
    bdCypher();
    bdCypher(bdUInt32 blockSize);
    ~bdCypher();
    void operator delete(void* p);
};