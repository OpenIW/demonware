// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

bdBandwidthTestResults::bdBandwidthTestResults()
    : m_bytesReceived(0), m_receivePeriodMs(1), m_avgSeqNumber(0), m_minSeqNumber(0), m_maxSeqNumber(0)
{
}

void bdBandwidthTestResults::clear()
{
    m_bytesReceived = 0;
    m_receivePeriodMs = 1;
    m_avgSeqNumber = 0;
    m_minSeqNumber = 0;
    m_maxSeqNumber = 0;
}

bdBool bdBandwidthTestResults::serialize(bdUByte8* const buffer, const bdUInt bufferSize)
{
    bdUInt offset = 0;
    bdBool ok = bdBytePacker::appendBasicType<bdUInt>(buffer, bufferSize, 0, offset, m_bytesReceived);
    ok = ok == bdBytePacker::appendBasicType<bdUInt>(buffer, bufferSize, offset, offset, m_receivePeriodMs);
    ok = ok == bdBytePacker::appendBasicType<bdUInt>(buffer, bufferSize, offset, offset, m_avgSeqNumber);
    ok = ok == bdBytePacker::appendBasicType<bdUInt>(buffer, bufferSize, offset, offset, m_minSeqNumber);
    ok = ok == bdBytePacker::appendBasicType<bdUInt>(buffer, bufferSize, offset, offset, m_maxSeqNumber);

    bdAssert(ok, "bdBandwidthTestResults failed to serialize.");
    return ok;
}

bdBool bdBandwidthTestResults::deserialize(const bdByteBufferRef buffer)
{
    bdBool ok = buffer->read<bdUInt>(m_bytesReceived);
    ok = ok == buffer->read<bdUInt>(m_receivePeriodMs);
    ok = ok == buffer->read<bdUInt>(m_avgSeqNumber);
    ok = ok == buffer->read<bdUInt>(m_minSeqNumber);
    ok = ok == buffer->read<bdUInt>(m_maxSeqNumber);

    bdAssert(ok, "bdBandwidthTestResults failed to deserialize.");
    return ok;
}

bdFloat32 bdBandwidthTestResults::getBandwidth()
{
    return ((8 * m_bytesReceived) / ((m_receivePeriodMs <= 1) ? 1 : m_receivePeriodMs) / 1000.0) / 1024.0;
}

void bdBandwidthTestResults::initializeFromBitsPerSecond(bdUInt bitsPerSecond)
{
    m_receivePeriodMs = 1000;
    m_avgSeqNumber = 500;
    m_minSeqNumber = 0;
    m_maxSeqNumber = 1000;
    m_bytesReceived = bitsPerSecond / 8;
}

