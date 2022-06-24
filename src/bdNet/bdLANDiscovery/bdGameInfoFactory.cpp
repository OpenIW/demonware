// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdNet/bdNet.h"

void* bdGameInfoFactoryImpl::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

void bdGameInfoFactoryImpl::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

bdGameInfoFactoryImpl::bdGameInfoFactoryImpl() : m_creator(NULL)
{
}

bdGameInfoFactoryImpl::~bdGameInfoFactoryImpl()
{
    if (m_creator)
    {
        delete m_creator;
    }
}

bdGameInfo* bdGameInfoFactoryImpl::create()
{
    return m_creator->create();
}

void bdGameInfoFactoryImpl::setClass(bdCreatorBase<bdGameInfo>* const creator)
{
    if (m_creator)
    {
        bdLogWarn("gameInfoFactory", "Game info class already set, BD_REGISTER_GAME_INFO_CLASS should only be called once otherwise memory is leaked.");
    }
    else
    {
        m_creator = creator;
    }
}
