// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdSummaryMetaHandler::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

bdSummaryMetaHandler::bdSummaryMetaHandler()
    : bdFileMetaData()
{
}

bdSummaryMetaHandler::~bdSummaryMetaHandler()
{
    delete this;
}

bdBool bdSummaryMetaHandler::deserialize(bdByteBufferRef buffer)
{
    return buffer->readString(m_url, sizeof(m_url)) && buffer->readUInt32(&m_fileSize);
}

bdUInt bdSummaryMetaHandler::sizeOf()
{
    return sizeof(bdSummaryMetaHandler);
}
