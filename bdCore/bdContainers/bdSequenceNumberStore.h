// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdSequenceNumberStore
{
public:
    enum bdSequenceStatus : __int32
    {
        BD_SN_INVALID_SMALLER = -2,
        BD_SN_VALID_SMALLER = -1,
        BD_SN_INVALID_DUPLICATE = 0,
        BD_SN_VALID_LARGER = 1,
        BD_SN_VALID_MUCH_LARGER = 2,
    };
protected:
    bdUInt32 m_bitmap;
    bdSequenceNumber m_lastSeq;
public:
    bdSequenceNumberStore();
    bdSequenceNumberStore(const bdSequenceNumber& initial);
    bdSequenceStatus check(const bdSequenceNumber& thisSeq);
    bdSequenceNumber& getLastSequenceNumber();
    void reset(const bdSequenceNumber& initial);
};
