// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdURL : public bdTaskResult
{
public:
    bdNChar8 m_url[384];
    bdUInt16 m_serverType;
    bdNChar8 m_serverIndex[128];
    bdUInt64 m_fileID;

    void operator delete(void* p);
    bdURL();
    ~bdURL();
    virtual bdBool deserialize(bdByteBufferRef buffer);
    virtual bdUInt sizeOf();
};
