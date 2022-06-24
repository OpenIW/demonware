// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdFileInfo::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdFileInfo::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdFileInfo::bdFileInfo()
    : bdTaskResult(), m_fileID(0), m_createTime(0), m_modifedTime(0), m_visibility(BD_MAX_VISIBILITY_TYPE), m_ownerID(0), m_fileSize(0)
{
    bdMemset(m_fileName, 0, sizeof(m_fileName));
}

bdFileInfo::~bdFileInfo()
{
}

bdBool bdFileInfo::deserialize(bdByteBufferRef buffer)
{
    bdBool ok = buffer->readUInt32(m_fileSize);
    ok = ok == buffer->readUInt64(m_fileID);
    ok = ok == buffer->readUInt32(m_createTime);
    ok = ok == buffer->readUInt32(m_modifedTime);

    bdBool visibility = false;
    ok = ok == buffer->readBool(visibility);
    m_visibility = static_cast<bdFileInfo::bdVisibility>(visibility);

    ok = ok == buffer->readUInt64(m_ownerID);
    ok = ok == buffer->readString(m_fileName, sizeof(m_fileName));
    return ok;
}

bdUInt bdFileInfo::sizeOf()
{
    return sizeof(bdFileInfo);
}
