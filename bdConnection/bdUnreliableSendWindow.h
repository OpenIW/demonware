// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdUnreliableSendWindow
{
protected:
    bdSeqNumber m_seqNumber;
    bdQueue<bdDataChunkRef> m_sendQueue;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdUnreliableSendWindow();
    virtual ~bdUnreliableSendWindow();
    void add(bdDataChunkRef chunk);
    void getDataToSend(bdPacket* packet);
    void reset();
};