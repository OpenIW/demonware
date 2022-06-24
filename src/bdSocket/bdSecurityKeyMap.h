// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdSecurityKeyMap
{
protected:
    bdSecurityKeyMapListener* m_listener;
    bdHashMap<bdSecurityID,bdSecurityKey,bdHashingClass> m_map;
public:
    bdSecurityKeyMap();
    ~bdSecurityKeyMap();
    bdBool registerKey(const bdSecurityID& id, const bdSecurityKey& key);
    bdBool unregisterKey(const bdSecurityID& id);
    bdBool get(const bdSecurityID& id, bdSecurityKey& key);
    bdBool contains(const bdSecurityID& id);
    void clear();
    void registerListener(bdSecurityKeyMapListener* listener);
    void unregisterListener();
    static bdBool initKey(bdSecurityID& id, bdSecurityKey& key);
};
