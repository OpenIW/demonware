// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdConnection/bdConnection.h"

bdConnectionStatistics::bdConnectionStatistics()
{
    reset();
}

void bdConnectionStatistics::reset()
{
    m_bytesSent = 0;
    m_bytesSentPerSecond = 0;
    m_lastBytesSent = 0;
    m_avgBytesSent = 0;
    m_bytesRecv = 0;
    m_avgPacketSentSize = 0;
    m_avgPacketRecvSize = 0;
    m_packetsSent = 0;
    m_packetsRecv = 0;
    m_maxPacketSizeSent = 0;
    m_maxPacketSizeRecv = 0;
    m_minPacketSizeSent = -1;
    m_minPacketSizeRecv = -1;
    m_maxRTT = 0.0;
    m_minRTT = 0.0;
    m_avgRTT = 0.0;
}

void bdConnectionStatistics::addBytesSent(const bdUInt bytes)
{
    m_bytesSent += bytes;
}

void bdConnectionStatistics::addBytesRecv(const bdUInt bytes)
{
    m_bytesRecv += bytes;
}

void bdConnectionStatistics::addPacketSizeSent(const bdUInt bytes)
{
    m_maxPacketSizeSent = m_maxPacketSizeSent > bytes ? m_maxPacketSizeSent : bytes;
    m_minPacketSizeSent = m_minPacketSizeSent < bytes ? m_minPacketSizeSent : bytes;
    m_avgPacketSentSize = m_avgPacketSentSize ? (bytes + m_avgPacketSentSize) / 2 : bytes;
}

void bdConnectionStatistics::addPacketSizeRecv(const bdUInt bytes)
{
    m_maxPacketSizeRecv = m_maxPacketSizeRecv > bytes ? m_maxPacketSizeRecv : bytes;
    m_minPacketSizeRecv = m_minPacketSizeRecv < bytes ? m_minPacketSizeRecv : bytes;
    m_avgPacketRecvSize = m_avgPacketRecvSize ? (bytes + m_avgPacketRecvSize) / 2 : bytes;
}

void bdConnectionStatistics::addPacketsSent(const bdUInt bytes)
{
    m_packetsSent += bytes;
}

void bdConnectionStatistics::addPacketsRecv(const bdUInt bytes)
{
    m_packetsRecv += bytes;
}

void bdConnectionStatistics::setLastRTT(const bdFloat32 time)
{
    if (m_maxRTT == 0.0 && m_minRTT == 0.0)
    {
        m_maxRTT = time;
        m_minRTT = time;
    }
    m_maxRTT = m_maxRTT <= time ? time : m_maxRTT;
    m_minRTT = m_minRTT >= time ? time : m_minRTT;
    m_avgRTT = time;
}

void bdConnectionStatistics::update(const bdFloat32 time)
{
    bdUInt bytesAverage;
    bdUInt bytesDifference;

    if (time > 0.0f)
    {
        bytesDifference = m_bytesSent - m_lastBytesSent;
        m_lastBytesSent = m_bytesSent;
        bytesAverage = (bytesDifference + m_avgBytesSent) / 2;
        m_avgBytesSent = bytesAverage;
        m_bytesSentPerSecond = (bytesAverage + 0) / time;
    }
}

bdUInt bdConnectionStatistics::getBytesSent() const
{
    return m_bytesSent;
}

bdUInt bdConnectionStatistics::getBytesRecv() const
{
    return m_bytesRecv;
}

bdUInt bdConnectionStatistics::getPacketsSent() const
{
    return m_packetsSent;
}

bdUInt bdConnectionStatistics::getPacketsRecv() const
{
    return m_packetsRecv;
}

bdFloat32 bdConnectionStatistics::getAvgRTT() const
{
    return m_avgRTT;
}
