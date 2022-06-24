// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdQoSRemoteAddr
{
public:
    bdCommonAddrRef m_addr;
    bdSecurityID m_id;
    bdSecurityKey m_key;

    bdQoSRemoteAddr();
    bdQoSRemoteAddr(const bdQoSRemoteAddr& other);
    bdQoSRemoteAddr(const bdCommonAddrRef& addr, const bdSecurityID& id, const bdSecurityKey& key);
    ~bdQoSRemoteAddr();
};
