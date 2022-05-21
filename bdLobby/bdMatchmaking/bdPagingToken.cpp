// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

void bdPagingToken::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdPagingToken::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdPagingToken::bdPagingToken(bdUInt numResultsPerPage)
    : bdTaskResultProcessor(), m_status(BD_NOT_STARTED), m_sessionID(), m_numResultsPerPage(numResultsPerPage)
{
}

bdPagingToken::~bdPagingToken()
{
}

void bdPagingToken::setRandomSessionID()
{
    bdSingleton<bdTrulyRandomImpl>::getInstance()->getRandomUByte8(m_sessionID.ab, sizeof(m_sessionID.ab));
    m_status = BD_IN_PROGRESS;
}

bdBool bdPagingToken::processResult(bdTaskResult* lastResultSet, bdUInt numResults)
{
    if (m_status == BD_FINISHED)
    {
        return false;
    }

    if (numResults >= m_numResultsPerPage && m_numResultsPerPage)
    {
        m_status = BD_IN_PROGRESS;
    }
    else
    {
        m_status = BD_FINISHED;
    }

    if (numResults)
    {
        // TODO
    }
    return true;
}

const bdSecurityID* bdPagingToken::getSessionID() const
{
    return &m_sessionID;
}

bdUInt bdPagingToken::getNumResultsPerPage() const
{
    return m_numResultsPerPage;
}

bdPagingToken::bdStatus bdPagingToken::getStatus() const
{
    return m_status;
}
