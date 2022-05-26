// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdLoopbackConnection : public bdConnection
{
protected:
    bdQueue<bdMessageRef> m_messages;
public:
    void* operator new(bdUWord nbytes);
    void operator delete(void* p);
    bdLoopbackConnection(bdCommonAddrRef addr);
    ~bdLoopbackConnection();
    bdUInt getDataToSend();
    bdBool getMessageToDispatch(bdMessageRef& message);
    bdBool receive(const bdUByte8*, const bdUInt);
    bdBool send(bdMessageRef message);
    void updateStatus();
};
