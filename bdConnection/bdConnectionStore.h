// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdConnectionStore
{
public:
    enum bdConnectionStoreShutdownType : bdInt
    {
        BD_CONNECTION_STORE_SHUTDOWN_ELEGANT = 0x0,
        BD_CONNECTION_STORE_SHUTDOWN_IMMEDIATE = 0x1,
    };
    enum bdConnectionStoreStatus : bdInt
    {
        BD_CONNECTION_STORE_UNINITIALIZED = 0x0,
        BD_CONNECTION_STORE_INITIALIZED = 0x1,
        BD_CONNECTION_STORE_SHUTTING_DOWN = 0x2,
        BD_CONNECTION_STORE_ERROR = 0x3,
    };
protected:
    bdSocketRouter* m_socket;
    bdHashMap<bdAddrHandleRefWrapper, bdReference<bdConnection>, bdAddrHandleRefWrapper> m_connectionMap;
    bdConnectionStore::bdConnectionStoreStatus m_status;
    bdDispatcher m_dispatcher;
    bdStopwatch m_shutdownTimer;
    bdAddressMap* m_addrMap;
    bdConnectionStoreConfig m_config;
public:
    bdConnectionStore();
    ~bdConnectionStore();
    bdBool init(bdSocketRouter* socket, const bdConnectionStoreConfig& config);
    bdBool flushAll();
    bdUInt flush(bdConnection& connection);
    bdBool startShutdown(const bdConnectionStore::bdConnectionStoreShutdownType shutdownType);
    bdBool receiveAll();
    void dispatchAll();
    void registerDispatchInterceptor(bdDispatchInterceptor* const interceptor);
    void unregisterDispatchInterceptor(bdDispatchInterceptor* const interceptor);
    bdConnectionStore::bdConnectionStoreStatus getStatus() const;
};
