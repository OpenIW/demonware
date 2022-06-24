// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdBandwidthTestResults
{
public:
    bdBandwidthTestResults();
    void clear();
    bdBool serialize(bdUByte8* const buffer, const bdUInt bufferSize);
    bdBool deserialize(const bdByteBufferRef buffer);
    bdFloat32 getBandwidth();
    void initializeFromBitsPerSecond(bdUInt bitsPerSecond);
    bdUInt m_bytesReceived;
    bdUInt m_receivePeriodMs;
    bdUInt m_avgSeqNumber;
    bdUInt m_minSeqNumber;
    bdUInt m_maxSeqNumber;
};
