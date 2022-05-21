// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdFileData::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdFileData::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdFileData::bdFileData(void* fileData, bdUInt fileSize)
    : bdTaskResult(), m_fileData(fileData), m_fileSize(fileSize)
{
}

bdBool bdFileData::deserialize(bdByteBufferRef buffer)
{
    return buffer->readBlob(reinterpret_cast<bdUByte8*>(m_fileData), &m_fileSize);
}

bdUInt bdFileData::sizeOf()
{
    return sizeof(bdFileData);
}
