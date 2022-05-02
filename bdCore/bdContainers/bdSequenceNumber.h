// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

typedef bdUInt16 bdSeqNumber;

class bdSequenceNumber
{
public:
    bdInt32 m_seqNum;

    bdSequenceNumber(const bdSequenceNumber* last, const bdUInt seqNumber, const bdUInt bits);
    bdSequenceNumber(const bdInt seqNum);
    bdSequenceNumber();

    bdBool operator!=(const bdSequenceNumber* other);
    bdSequenceNumber operator+(const bdSequenceNumber* other);
    bdSequenceNumber* operator++();
    bdSequenceNumber* operator+=(const bdSequenceNumber* other);
    bdSequenceNumber operator-(const bdSequenceNumber* other);
    bdBool operator<(const bdSequenceNumber* other);
    bdBool operator<=(const bdSequenceNumber* other);
    bdBool operator==(const bdSequenceNumber* other);
    bdBool operator>(const bdSequenceNumber* other);
    bdBool operator>=(const bdSequenceNumber* other);

    bdInt32 getValue();
    void set(const bdSequenceNumber* last, const bdUInt seqNumber, const bdUInt bits);
};