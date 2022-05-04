// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdDispatchInterceptor
{
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    virtual ~bdDispatchInterceptor();
    virtual bdBool accept(const bdReceivedMessage* __formal) { return 0; };
};

class bdDispatcher
{
protected:
    bdFastArray<bdDispatchInterceptor*> m_interceptors;
public:
    bdDispatcher();
    void process(bdConnectionRef connection);
    void registerInterceptor(bdDispatchInterceptor* const interceptor);
    void unregisterInterceptor(bdDispatchInterceptor* const interceptor);
    void reset();
};