// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdReliableSendWindow
{
public:
    enum bdCongestionWindowDecreaseReason : bdInt
    {
        BD_CWDR_PACKET_LOSS_DETECTED = 0x0,
        BD_CWDR_RESEND_TIMER_EXPIRED = 0x1,
        BD_CWDR_INACTIVE = 0x2,
    };
    class bdMessageFrame
    {
    public:
        bdDataChunkRef m_chunk;
        bdStopwatch m_timer;
        bdUByte8 m_sendCount;
        bdUByte8 m_missingCount;
        bdBool m_gapAcked;

        bdMessageFrame();
        bdMessageFrame(bdDataChunkRef chunk);
        bdMessageFrame* operator=(bdMessageFrame* other);
    };
protected:
    bdSequenceNumber m_lastAcked;
    bdSequenceNumber m_nextFree;
    bdFloat32 m_timeoutPeriod;
    bdUByte8 m_retransmitCountThreshold;
    bdReliableSendWindow::bdMessageFrame m_frame[128];
    bdWord m_remoteReceiveWindowCredit;
    bdWord m_flightSize;
    bdWord m_partialBytesAcked;
    bdWord m_slowStartThresh;
    bdWord m_congestionWindow;
    bdStopwatch m_lastSent;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdReliableSendWindow();
    bdBool add(bdDataChunkRef chunk);
    void getDataToSend(bdPacket* packet);
    void decreaseCongestionWindow(const bdReliableSendWindow::bdCongestionWindowDecreaseReason reason);
    void increaseCongestionWindow(const bdUWord bytesAcked);
    bdBool handleAck(bdSAckChunkRef chunk, bdFloat32* rtt);
    bdBool isEmpty();
    void setTimeoutPeriod(const bdFloat32 secs);
    const bdFloat32 getTimeoutPeriod() const;
};