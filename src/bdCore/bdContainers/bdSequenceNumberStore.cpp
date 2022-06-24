// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdCore/bdCore.h"

bdSequenceNumberStore::bdSequenceNumberStore()
{
    m_bitmap = 0;
    m_lastSeq.m_seqNum = 0;
}

bdSequenceNumberStore::bdSequenceNumberStore(const bdSequenceNumber& initial)
{
    m_bitmap = 0;
    m_lastSeq.m_seqNum = initial.m_seqNum;
}

bdSequenceNumberStore::bdSequenceStatus bdSequenceNumberStore::check(const bdSequenceNumber& thisSeq)
{
    bdUInt diff;
    bdSequenceNumber sdiff;

    if (thisSeq > m_lastSeq)
    {
        sdiff = thisSeq - m_lastSeq;
        diff = sdiff.getValue();
        if (diff >= 32)
        {
            m_lastSeq = thisSeq;
            m_bitmap = 1;
            return BD_SN_VALID_MUCH_LARGER;
        }
        else
        {
            m_bitmap = (m_bitmap << diff) | 1;
            m_lastSeq = thisSeq;
            return BD_SN_VALID_LARGER;
        }
    }
    else
    {
        sdiff = m_lastSeq - thisSeq;
        diff = sdiff.getValue();
        if (diff < 0x20)
        {
            if (((1 << diff) & m_bitmap) != 0)
            {
                return BD_SN_INVALID_DUPLICATE;
            }
            else
            {
                m_bitmap |= 1 << diff;
                return BD_SN_VALID_SMALLER;
            }
        }
        else
        {
            return BD_SN_INVALID_SMALLER;
        }
    }
}

bdSequenceNumber& bdSequenceNumberStore::getLastSequenceNumber()
{
    return m_lastSeq;
}

void bdSequenceNumberStore::reset(const bdSequenceNumber& initial)
{
    m_bitmap = 0;
    m_lastSeq.m_seqNum = initial.m_seqNum;
}
