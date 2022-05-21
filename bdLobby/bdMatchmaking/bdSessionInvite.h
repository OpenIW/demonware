// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdSessionInvite : public bdTaskResult
{
public:
    bdUInt64 m_senderID;
    bdNChar8 m_senderName[64];
    bdSessionID m_sessionID;
    bdUByte8 m_attachment[1024];
    bdUInt32 m_attachmentSize;

    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdSessionInvite();
    ~bdSessionInvite();
    virtual bdBool deserialize(bdByteBufferRef buffer);
    virtual bdUInt sizeOf();
};
