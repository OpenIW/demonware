// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocketRouter.h"

bdServiceBandwidthArbitrator::bdServiceBandwidthArbitrator() : m_allowedSend(0), m_sentThisSlice(false)
{
}

void bdServiceBandwidthArbitrator::addSliceQuota(bdUInt bytes)
{
    if (m_allowedSend / bytes < 2 && (m_allowedSend <= 0 || m_sentThisSlice))
    {
        m_allowedSend += bytes;
    }
    m_sentThisSlice = false;
}

bdBool bdServiceBandwidthArbitrator::allowedSend(bdUInt __formal)
{
    return m_allowedSend > 0;
}

void bdServiceBandwidthArbitrator::reset()
{
    m_allowedSend = 0;
    m_sentThisSlice = 0;
}

void bdServiceBandwidthArbitrator::send(bdUInt bytes)
{
    m_allowedSend -= bytes;
    m_sentThisSlice = true;
}
