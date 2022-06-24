// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdGameInfoFactoryImpl
{
protected:
    bdCreatorBase<bdGameInfo>* m_creator;
public:
    void* operator new(bdUWord nbytes);
    void operator delete(void* p);
    bdGameInfoFactoryImpl();
    ~bdGameInfoFactoryImpl();
    bdGameInfo* create();
    void setClass(bdCreatorBase<bdGameInfo>* const creator);
};

class bdGameInfoFactory : bdSingleton<bdGameInfoFactoryImpl> {};