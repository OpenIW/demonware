// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdCore/bdCore.h"

bdSingletonRegistryImpl::bdSingletonRegistryImpl()
    : m_destroyFunctions(0u), m_cleaningUp(false)
{
}

bdSingletonRegistryImpl::~bdSingletonRegistryImpl()
{
}

void bdSingletonRegistryImpl::cleanUp()
{
    void (**end)(void);
    void (**begin)(void);

    begin = m_destroyFunctions.begin();
    end = m_destroyFunctions.end();
    m_cleaningUp = true;

    while (end != begin)
    {
        (*--end)();
    }
}

bdBool bdSingletonRegistryImpl::add(const bdSingletonDestroyFunction destroyFunction)
{
    bdBool added = !m_cleaningUp;
    if (added)
    {
        m_destroyFunctions.pushBack(destroyFunction);
    }
    return added;
}

bdSingletonRegistryImpl* bdSingleton<bdSingletonRegistryImpl>::m_instance = NULL;
bdTrulyRandomImpl* bdSingleton<bdTrulyRandomImpl>::m_instance = NULL;
