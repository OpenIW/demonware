// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdMatchMakingInfo : public bdTaskResult
{
public:
    bdSecurityID m_sessionID;
    bdUByte8 m_hostAddr[255];
    bdUInt m_hostAddrSize;
    bdUInt m_gameType;
    bdUInt m_maxPlayers;
    bdUInt m_numPlayers;

    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdMatchMakingInfo();
    ~bdMatchMakingInfo();
    virtual bdBool deserialize(bdByteBufferRef buffer);
    virtual bdUInt sizeOf();
    virtual void serialize(bdByteBuffer& buffer);
    bdCommonAddrRef getHostAddrAsCommonAddr(bdCommonAddrRef localCommonAddr);
    void setHostAddr(bdCommonAddrRef localCommonAddr);
};
