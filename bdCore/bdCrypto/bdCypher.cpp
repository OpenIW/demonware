// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdCore/bdCore.h"

bdCypher::bdCypher()
{
    m_blockSize = 0;
}

bdCypher::bdCypher(bdUInt32 blockSize)
{
    m_blockSize = blockSize;
    bdAssert((blockSize <= BD_CYPHER_BLOCK_SIZE_MAX), "BD_CYPHER_BLOCK_SIZE_MAX too small for cypher of block size[%d]", blockSize);
}

bdCypher::~bdCypher()
{
}

void bdCypher::operator delete(void* p)
{
    bdMemory::deallocate(p);
}
