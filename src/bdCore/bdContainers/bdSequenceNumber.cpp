// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdCore/bdCore.h"

bdSequenceNumber::bdSequenceNumber(const bdSequenceNumber& last, const bdUInt seqNumber, const bdUInt bits)
{
    m_seqNum = -1;
    set(last, seqNumber, bits);
}

bdSequenceNumber::bdSequenceNumber(const bdInt seqNum)
{
    m_seqNum = seqNum;
}

bdSequenceNumber::bdSequenceNumber()
{
    m_seqNum = 0;
}

bdBool bdSequenceNumber::operator!=(const bdSequenceNumber& other)
{
    return (bdBool)(m_seqNum != other.m_seqNum);
}

bdSequenceNumber bdSequenceNumber::operator+(const bdSequenceNumber& other)
{
    bdSequenceNumber seqNum(m_seqNum);

    seqNum.m_seqNum += other.m_seqNum;
    return seqNum;
}

bdSequenceNumber* bdSequenceNumber::operator++()
{
    m_seqNum++;
    return this;
}

bdSequenceNumber* bdSequenceNumber::operator+=(const bdSequenceNumber& other)
{
    m_seqNum += other.m_seqNum;
    return this;
}

bdSequenceNumber bdSequenceNumber::operator-(const bdSequenceNumber& other) const
{
    bdSequenceNumber seqNum(m_seqNum);

    seqNum.m_seqNum -= other.m_seqNum;
    return seqNum;
}

bdBool bdSequenceNumber::operator<(const bdSequenceNumber& other) const
{
    return m_seqNum < other.m_seqNum;
}

bdBool bdSequenceNumber::operator<=(const bdSequenceNumber& other) const
{
    return m_seqNum <= other.m_seqNum;
}

bdBool bdSequenceNumber::operator==(const bdSequenceNumber& other) const
{
    return m_seqNum == other.m_seqNum;
}

bdBool bdSequenceNumber::operator>(const bdSequenceNumber& other) const
{
    return other.m_seqNum < m_seqNum;
}

bdBool bdSequenceNumber::operator>=(const bdSequenceNumber& other) const
{
    return other.m_seqNum <= m_seqNum;
}

bdInt32 bdSequenceNumber::getValue()
{
    return m_seqNum;
}

void bdSequenceNumber::set(const bdSequenceNumber& last, const bdUInt seqNumber, const bdUInt bits)
{
    bdBool lastLess, lastGreater;
    bdInt rangeChange;
    bdInt curSeq;
    bdInt lastRangeBase;
    bdInt lastSeq;
    bdInt range;

    range = 2 << (bits - 1);
    lastSeq = last.m_seqNum % range;
    lastRangeBase = last.m_seqNum - lastSeq;
    curSeq = seqNumber % range;
    bdAssert(static_cast<bdInt>(seqNumber) < range, "Sequence number given outside the range.");
    if (lastSeq >= 0)
    {
        if (lastSeq == curSeq)
        {
            m_seqNum = last.m_seqNum;
        }
        else
        {
            if (lastSeq >= curSeq || (lastGreater = 1, curSeq - lastSeq >= range / 2))
            {
                lastGreater = 0;
                if (lastSeq > curSeq)
                {
                    lastGreater = lastSeq - curSeq > range / 2;
                }
            }
            if (lastSeq >= curSeq || (lastLess = 1, curSeq - lastSeq <= range / 2))
            {
                lastLess = 0;
                if (lastSeq > curSeq)
                {
                    lastLess = lastSeq - curSeq < range / 2;
                }
            }
            rangeChange = 0;
            if (!lastLess && !lastGreater)
            {
                bdLogWarn("bdCore/bdContainers/sequenceNumber", "Sequence numbers are too far away and cannot be compared.");
            }
            if (!lastLess || lastSeq <= curSeq)
            {
                if (lastLess && lastSeq < curSeq)
                {
                    rangeChange = -1;
                }
                else if (lastGreater && lastSeq > curSeq)
                {
                    rangeChange = 1;
                }
            }
            m_seqNum = curSeq + rangeChange * range + lastRangeBase;
        }
    }
    else
    {
        m_seqNum = seqNumber;
    }
}
