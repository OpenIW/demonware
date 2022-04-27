// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocket.h"

bdSecurityKeyMap::bdSecurityKeyMap() : m_map(4u, 0.75f), m_listener(NULL)
{
}

bdSecurityKeyMap::~bdSecurityKeyMap()
{
}

bdBool bdSecurityKeyMap::registerKey(const bdSecurityID* id, const bdSecurityKey* key)
{
    bdNChar8 tmp[36];
    bdBool added;

    added = m_map.put(id, key);
    bdSecurityInfo::toString(id, tmp, sizeof(tmp));
    bdLogInfo("bdSocket/bdSecurityKeyMap", "Putting bdSecurityID : %s", tmp);
    bdSecurityInfo::toString(key, tmp, sizeof(tmp));
    bdLogInfo("bdSocket/bdSecurityKeyMap", "Putting bdSecurityKey: %s", tmp);
    if (!added)
    {
        bdLogInfo("bdSocket/bdSecurityKeyMap", "bdSecurityID already present.");
    }
    return added;
}

bdBool bdSecurityKeyMap::unregisterKey(const bdSecurityID* id)
{
    bdBool removed;
    bdNChar8 tmp[36];

    if (m_listener)
    {
        if (m_map.containsKey(id))
        {
            m_listener->onSecurityKeyRemove(id);
        }
    }
    removed = m_map.remove(id);
    bdSecurityInfo::toString(id, tmp, sizeof(tmp));
    bdLogInfo("bdSocket/bdSecurityKeyMap", "Removing bdSecurityID: %s", tmp);
    if (!removed)
    {
        bdLogInfo("bdSocket/bdSecurityKeyMap", "bdSecurityID not present.");
    }
    return removed;
}

bdBool bdSecurityKeyMap::get(const bdSecurityID* id, bdSecurityKey* key)
{
    bdBool found;
    bdNChar8 tmp[36];

    found = m_map.get(id, key);
    bdSecurityInfo::toString(id, tmp, sizeof(tmp));
    bdLogInfo("bdSocket/bdSecurityKeyMap", "Searching for bdSecurityID: %s", tmp);
    if (found)
    {
        bdSecurityInfo::toString(key, tmp, sizeof(tmp));
        bdLogInfo("bdSocket/bdSecurityKeyMap", "Found bdSecurityKey: %s", tmp);
    }
    else
    {
        bdLogInfo("bdSocket/bdSecurityKeyMap", "bdSecurityID not present");
    }
    return found;
}

bdBool bdSecurityKeyMap::contains(const bdSecurityID* id)
{
    bdBool found;
    bdNChar8 tmp[36];

    found = m_map.containsKey(id);
    bdSecurityInfo::toString(id, tmp, sizeof(tmp));
    if (found)
    {
        bdLogInfo("bdSocket/bdSecurityKeyMap", "Found bdSecurityID: [%s]", tmp);
    }
    else
    {
        bdLogInfo("bdSocket/bdSecurityKeyMap", "bdSecurityID: [%s] not present", tmp);
    }
    return found;
}

void bdSecurityKeyMap::clear()
{
    bdHashMap<bdSecurityID,bdSecurityKey,bdHashingClass>::Iterator it;

    if (m_listener)
    {
        for (it = m_map.getIterator(); it; m_map.next(&it))
        {
            m_listener->onSecurityKeyRemove(m_map.getKey(it));
        }
    }
    m_map.clear();
}

void bdSecurityKeyMap::registerListener(bdSecurityKeyMapListener* listener)
{
    bdAssert(m_listener == BD_NULL, "Only one listener is allowed at once.");
    m_listener = listener;
}

void bdSecurityKeyMap::unregisterListener()
{
    m_listener = NULL;
}

bdBool bdSecurityKeyMap::initKey(bdSecurityID* id, bdSecurityKey* key)
{
    bdNChar8 tmp[36];

    bdSingleton<bdTrulyRandomImpl>::getInstance()->getRandomUByte8(reinterpret_cast<byte*>(id), sizeof(bdSecurityID));
    bdSingleton<bdTrulyRandomImpl>::getInstance()->getRandomUByte8(key->ab, sizeof(key->ab));
    bdSecurityInfo::toString(id, tmp, sizeof(tmp));
    bdLogInfo("bdSocket/bdSecurityKeyMap", "Created bdSecurityID: %s", tmp);
    bdSecurityInfo::toString(key, tmp, sizeof(tmp));
    bdLogInfo("bdSocket/bdSecurityKeyMap", "Created bdSecurityKey: %s", tmp);
    return true;
}
