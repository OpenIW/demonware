// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

bdFileMetaData::bdFileMetaData()
    : bdTaskResult(), m_fileID(0), m_createTime(0), m_modifedTime(0), m_fileSize(0), m_ownerID(0), m_fileSlot(0), m_numTags(0), m_metaDataSize(0), m_summaryFileSize(0)
{
    resetArrays();
}

bdFileMetaData::~bdFileMetaData()
{
}

void bdFileMetaData::reset()
{
    m_fileID = 0;
    m_fileSlot = 0;
    m_createTime = 0;
    m_modifedTime = 0;
    m_fileSize = 0;
    m_ownerID = 0;
    m_numTags = 0;
    m_metaDataSize = 0;
    m_summaryFileSize = 0;
    resetArrays();
}

void bdFileMetaData::resetArrays()
{
    bdMemset(m_fileName, 0, sizeof(m_fileName));
    bdMemset(m_url, 0, sizeof(m_url));;
    bdMemset(m_ownerName, 0, sizeof(m_ownerName));
    bdMemset(m_metaData, 0, sizeof(m_metaData));
    for (bdUInt i = 0; i < m_numTags; ++i)
    {
        m_tags[i].set(0, 0);
    }
}

bdBool bdFileMetaData::deserialize(bdByteBufferRef buffer)
{
    reset();
    bdBool ok = buffer->readUInt32(&m_createTime);
    ok = ok == buffer->readUInt32(&m_modifedTime);
    ok = ok == buffer->readUInt32(&m_fileSize);
    ok = ok == buffer->readUInt64(&m_ownerID);
    ok = ok == buffer->readString(m_ownerName, sizeof(m_ownerName));
    ok = ok == buffer->readUInt16(&m_fileSlot);
    ok = ok == buffer->readString(m_fileName, sizeof(m_fileName));
    ok = ok == buffer->readString(m_url, sizeof(m_url));
    m_metaDataSize = 512;
    ok = ok == buffer->readBlob(m_metaData, &m_metaDataSize);
    ok = ok == buffer->readUInt32(&m_summaryFileSize);
    ok = ok == buffer->readArrayStart(10, &m_numTags);
    if (m_numTags)
        m_numTags >>= 1;
    for (bdUInt i = 0; i < m_numTags && ok; ++i)
    {
        ok = ok == m_tags[i].deserialize(buffer);
    }
    buffer->readArrayEnd();
    return ok;
}

bdUInt bdFileMetaData::sizeOf()
{
    return sizeof(bdFileMetaData);
}

bdFileMetaData* bdFileMetaData::operator=(const bdFileMetaData* other)
{
    m_fileID = other->m_fileID;
    m_createTime = other->m_createTime;
    m_modifedTime = other->m_modifedTime;
    m_fileSize = other->m_fileSize;
    m_fileID = other->m_fileID;
    for (bdUInt i = 0; i < sizeof(m_ownerName); ++i)
    {
        m_ownerName[i] = other->m_ownerName[i];
    }
    m_fileSlot = other->m_fileSlot;
    for (bdUInt i = 0; i < sizeof(m_fileName); ++i)
    {
        m_fileName[i] = other->m_fileName[i];
    }
    for (bdUInt i = 0; i < sizeof(m_url); ++i)
    {
        m_url[i] = other->m_url[i];
    }
    m_category = other->m_category;
    m_numTags = other->m_numTags;
    for (bdUInt i = 0; i < sizeof(m_metaData); ++i)
    {
        m_metaData[i] = other->m_metaData[i];
    }
    m_metaDataSize = other->m_metaDataSize;
    m_summaryFileSize = other->m_summaryFileSize;
    for (bdUInt i = 0; i < 40; ++i)
    {
        m_tags[i] = other->m_tags[i];
    }
    return this;
}
