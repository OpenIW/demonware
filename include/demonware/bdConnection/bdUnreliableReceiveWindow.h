// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdUnreliableReceiveWindow
{
protected:
    bdSequenceNumber m_seqNumber;
    bdQueue<bdDataChunkRef> m_recvQueue;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdUnreliableReceiveWindow();
    virtual ~bdUnreliableReceiveWindow();
    bdBool add(bdDataChunkRef chunk);
    bdDataChunkRef getNextToRead();
    void reset();
};