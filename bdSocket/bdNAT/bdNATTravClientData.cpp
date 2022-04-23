// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocketRouter.h"

bdNATTravClientData::bdNATTravClientData() 
    : m_state(BD_NTCDS_INITIAL), m_local(), m_remote(), m_listener(NULL), m_secondaryListener(NULL), m_tries(0), m_lastSent(), m_throttled(false), m_age()
{
}

bdNATTravClientData::bdNATTravClientData(const bdNATTravClientData* other) : m_state(other->m_state), m_local(other->m_local), m_remote(other->m_remote), m_listener(other->m_listener)
{
}

bdNATTravClientData::bdNATTravClientData(bdCommonAddrRef local, bdCommonAddrRef remote, bdNATTravListener* listener) 
    : m_state(BD_NTCDS_STAGE_1), m_local(local.m_ptr), m_remote(remote.m_ptr), m_listener(listener), m_secondaryListener(NULL), m_tries(0), m_lastSent(), m_throttled(false), m_age()
{
}

bdNATTravClientData::~bdNATTravClientData()
{
}

void bdNATTravClientData::callOnNATAddrDiscovery(bdCommonAddrRef remote, const bdAddr* realAddr)
{
    m_listener->onNATAddrDiscovery(remote.m_ptr, realAddr);
    if (m_secondaryListener)
    {
        m_secondaryListener->onNATAddrDiscovery(remote.m_ptr, realAddr);
    }
}

void bdNATTravClientData::callOnNATAddrDiscoveryFailed(bdCommonAddrRef remote)
{
    m_listener->onNATAddrDiscoveryFailed(remote.m_ptr);
    if (m_secondaryListener)
    {
        m_secondaryListener->onNATAddrDiscoveryFailed(remote.m_ptr);
    }
}

bdNATTravClientData* bdNATTravClientData::operator=(bdNATTravClientData* other)
{
    m_state = other->m_state;
    m_local = other->m_local;
    m_remote = other->m_remote;
    bdMemcpy(&m_listener, &other->m_listener, sizeof(m_listener));
    return this;
}
