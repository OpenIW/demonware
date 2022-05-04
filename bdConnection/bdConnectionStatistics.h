// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdConnectionStatistics
{
protected:
    bdUInt m_bytesSent;
    bdUInt m_bytesSentPerSecond;
    bdUInt m_avgBytesSent;
    bdUInt m_lastBytesSent;
    bdUInt m_bytesRecv;
    bdUInt m_avgPacketSentSize;
    bdUInt m_avgPacketRecvSize;
    bdUInt m_maxPacketSizeSent;
    bdUInt m_minPacketSizeSent;
    bdUInt m_maxPacketSizeRecv;
    bdUInt m_minPacketSizeRecv;
    bdUInt m_packetsSent;
    bdUInt m_packetsRecv;
    bdFloat32 m_maxRTT;
    bdFloat32 m_minRTT;
    bdFloat32 m_avgRTT;
public:
    bdConnectionStatistics();
    void reset();
    void addBytesSent(const bdUInt bytes);
    void addBytesRecv(const bdUInt bytes);
    void addPacketSizeSent(const bdUInt bytes);
    void addPacketSizeRecv(const bdUInt bytes);
    void addPacketsSent(const bdUInt bytes);
    void addPacketsRecv(const bdUInt bytes);
    void setLastRTT(const bdFloat32 time);
    void update(const bdFloat32 time);

    bdUInt getBytesSent() const;
    bdUInt getBytesRecv() const;
    bdUInt getPacketsSent() const;
    bdUInt getPacketsRecv() const;
    bdFloat32 getAvgRTT() const;
};
