// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdPagingToken : public bdTaskResultProcessor
{
public:
    enum bdStatus : bdInt
    {
        BD_NOT_STARTED = 0x0,
        BD_IN_PROGRESS = 0x1,
        BD_FINISHED = 0x2,
    };
protected:
    bdPagingToken::bdStatus m_status;
    bdSecurityID m_sessionID;
    bdUInt m_numResultsPerPage;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdPagingToken(bdUInt numResultsPerPage);
    ~bdPagingToken();
    void setRandomSessionID();
    virtual bdBool processResult(bdTaskResult* lastResultSet, bdUInt numResults);
    const bdSecurityID& getSessionID() const;
    bdUInt getNumResultsPerPage() const;
    bdPagingToken::bdStatus getStatus() const;
};
