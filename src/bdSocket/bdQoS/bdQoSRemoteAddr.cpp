// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocket.h"

bdQoSRemoteAddr::bdQoSRemoteAddr() 
    : m_addr(), m_id(), m_key()
{
}

bdQoSRemoteAddr::bdQoSRemoteAddr(const bdQoSRemoteAddr& other) 
    : m_addr(other.m_addr), m_id(other.m_id), m_key(other.m_key)
{
}

bdQoSRemoteAddr::bdQoSRemoteAddr(const bdCommonAddrRef& addr, const bdSecurityID& id, const bdSecurityKey& key) 
    : m_addr(addr), m_id(id), m_key(key)
{
}

bdQoSRemoteAddr::~bdQoSRemoteAddr()
{
}
