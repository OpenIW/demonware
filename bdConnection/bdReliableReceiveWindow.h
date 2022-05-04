// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdReliableReceiveWindow
{
protected:
    bdSequenceNumber m_newest;
    bdSequenceNumber m_lastCumulative;
    bdSequenceNumber m_lastDispatched;
    bdDataChunkRef m_frame[128];
    bdBool m_shouldAck;
    bdWord m_recvWindowCredit;
    bdWord m_recvWindowUsedCredit;
    bdSAckChunkRef m_sack;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdReliableReceiveWindow();
    bdBool add(bdDataChunkRef chunk);
    void calculateAck();
    void getDataToSend(bdPacket* packet);
    bdDataChunkRef getNextToRead();
};