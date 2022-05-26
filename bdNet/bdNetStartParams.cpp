// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdNet/bdNet.h"

bdNetStartParams::bdNetStartParams()
    : m_onlineGame(false), m_gamePort(3074), m_socket(NULL), m_natTravHosts(0u), m_localAddresses(0u), m_hostNameLookupConfig(), m_UPnPConfig(), m_useAnyIP(true), m_natTravPort(3074)
{
    m_natTravHosts.pushBack(bdString("stun.us.demonware.net"));
    m_natTravHosts.pushBack(bdString("stun.eu.demonware.net"));
    m_natTravHosts.pushBack(bdString("stun.jp.demonware.net"));
    m_natTravHosts.pushBack(bdString("stun.au.demonware.net"));
}

bdNetStartParams::bdNetStartParams(const bdNetStartParams& other)
    : m_onlineGame(other.m_onlineGame), m_gamePort(other.m_gamePort), m_socket(other.m_socket), m_natTravHosts(other.m_natTravHosts), m_natTravPort(other.m_natTravPort), 
    m_localAddresses(other.m_localAddresses), m_hostNameLookupConfig(other.m_hostNameLookupConfig), m_UPnPConfig(other.m_UPnPConfig), m_useAnyIP(other.m_useAnyIP)
{
}

bdNetStartParams::~bdNetStartParams()
{
}

bdNetStartParams& bdNetStartParams::operator=(const bdNetStartParams& other)
{
    m_onlineGame = other.m_onlineGame;
    m_gamePort = other.m_gamePort;
    m_socket = other.m_socket;
    m_natTravHosts = other.m_natTravHosts;
    m_natTravPort = other.m_natTravPort;
    m_localAddresses = other.m_localAddresses;
    m_hostNameLookupConfig = other.m_hostNameLookupConfig;
    m_UPnPConfig = other.m_UPnPConfig;
    return *this;
}
