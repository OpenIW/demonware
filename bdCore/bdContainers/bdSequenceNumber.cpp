/*
* DemonWare
* Copyright (c) 2020-2022 OpenIW
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, version 3.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "bdPlatform/bdPlatform.h"
#include "bdPlatform/bdPlatformLog/bdPlatformLog.h"

#include "bdSequenceNumber.h"

bdSequenceNumber::bdSequenceNumber(const bdSequenceNumber* last, const bdUInt seqNumber, const bdUInt bits)
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

bdBool bdSequenceNumber::operator!=(const bdSequenceNumber* other)
{
    return (bdBool)(m_seqNum != other->m_seqNum);
}

bdSequenceNumber bdSequenceNumber::operator+(const bdSequenceNumber* other)
{
    bdSequenceNumber seqNum(m_seqNum);

    seqNum.m_seqNum += other->m_seqNum;
    return seqNum;
}

bdSequenceNumber* bdSequenceNumber::operator++()
{
    m_seqNum++;
    return this;
}

bdSequenceNumber* bdSequenceNumber::operator+=(const bdSequenceNumber* other)
{
    m_seqNum += other->m_seqNum;
    return this;
}

bdSequenceNumber bdSequenceNumber::operator-(const bdSequenceNumber* other)
{
    bdSequenceNumber seqNum(m_seqNum);

    seqNum.m_seqNum -= other->m_seqNum;
    return seqNum;
}

bdBool bdSequenceNumber::operator<(const bdSequenceNumber* other)
{
    return (bdBool)(this->m_seqNum < other->m_seqNum);
}

bdBool bdSequenceNumber::operator<=(const bdSequenceNumber* other)
{
    return (bdBool)(this->m_seqNum <= other->m_seqNum);
}

bdBool bdSequenceNumber::operator==(const bdSequenceNumber* other)
{
    return (bdBool)(this->m_seqNum == other->m_seqNum);
}

bdBool bdSequenceNumber::operator>(const bdSequenceNumber* other)
{
    return (bdBool)(other->m_seqNum < this->m_seqNum);
}

bdBool bdSequenceNumber::operator>=(const bdSequenceNumber* other)
{
    return (bdBool)(other->m_seqNum <= this->m_seqNum);
}

bdInt32 bdSequenceNumber::getValue()
{
    return m_seqNum;
}

void bdSequenceNumber::set(const bdSequenceNumber* last, const bdUInt seqNumber, const bdUInt bits)
{
    bdBool lastLess, lastGreater;
    bdInt rangeChange;
    bdInt curSeq;
    bdInt lastRangeBase;
    bdInt lastSeq;
    bdInt range;

    range = 2 << (bits - 1);
    lastSeq = last->m_seqNum % range;
    lastRangeBase = last->m_seqNum - lastSeq;
    curSeq = static_cast<bdInt>(seqNumber) % range;
    //bdAssertMsg(static_cast<bdInt>(seqNumber) < rang, "Sequence number given outside the range.");
    if (lastSeq >= 0)
    {
        if (lastSeq == curSeq)
        {
            this->m_seqNum = last->m_seqNum;
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
                bdLogMessage(
                    BD_LOG_WARNING,
                    "warn/",
                    "bdCore/bdContainers/sequenceNumber",
                    __FILE__,
                    "set",
                    __LINE__,
                    "Sequence numbers are too far away and cannot be compared.");
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
            this->m_seqNum = curSeq + rangeChange * range + lastRangeBase;
        }
    }
    else
    {
        this->m_seqNum = seqNumber;
    }
}
