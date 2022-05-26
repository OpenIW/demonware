// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdConnection/bdConnection.h"

bdConnectionStore::bdConnectionStore()
    : m_socket(NULL), m_connectionMap(4u, 0.75f), m_status(BD_CONNECTION_STORE_UNINITIALIZED), m_dispatcher(), m_shutdownTimer(), m_addrMap(NULL), m_config()
{
}

bdConnectionStore::~bdConnectionStore()
{
    m_connectionMap.clear();
    m_dispatcher.reset();
}

bdBool bdConnectionStore::init(bdSocketRouter* socket, const bdConnectionStoreConfig& config)
{
    if (m_status)
    {
        bdLogWarn("bdConnection/connectionstore", "init() called multiple times.");
        return false;
    }
    if (!socket)
    {
        bdLogError("bdConnection/connectionstore", "A valid socket router pointer is required by this class");
        return false;
    }
    m_socket = socket;
    m_addrMap = &socket->getAddressMap();
    m_config = config;
    m_status = BD_CONNECTION_STORE_INITIALIZED;
    return true;

}

bdBool bdConnectionStore::flushAll()
{
    bdNChar8 addrStr[22];
    bdBool success;

    if (m_status != BD_CONNECTION_STORE_INITIALIZED && m_status != BD_CONNECTION_STORE_SHUTTING_DOWN)
    {
        bdLogWarn("bdConnection/connectionstore", "Cannot call flushAll unless class is initialized.");
        return false;
    }
    bdQueue<bdConnectionRef> toDisconnect;
    for (void* Iterator = m_connectionMap.getIterator(); Iterator; m_connectionMap.next(Iterator))
    {
        bdConnectionRef connection(m_connectionMap.getValue(Iterator));
        if (!connection->getAddress()->isLoopback())
        {
            bdUInt flushIters;
            switch (m_socket->getStatus(connection->getAddressHandle()))
            {

            case BD_SOCKET_IDLE:
                bdAddressMap::addrToString(connection->getAddressHandle(), addrStr, sizeof(addrStr));
                bdLogWarn("bdConnection/connectionstore", "SocketRouter reports socket idle. Disconnecting %s.", addrStr);
                toDisconnect.enqueue(&connection);
                break;
            case BD_SOCKET_CONNECTED:
                for (flushIters = 0; flushIters < m_config.m_maxFlushIterations; ++flushIters)
                {
                    if (!flush(**connection))
                    {
                        break;
                    }
                }
                if (connection->getStatus() != bdConnection::BD_DISCONNECTED)
                {
                    break;
                }
                bdAddressMap::addrToString(connection->getAddressHandle(), addrStr, sizeof(addrStr));
                bdLogInfo("bdConnection/connectionstore", "Connection state disconnected. Disconnecting %s.", addrStr);
                toDisconnect.enqueue(&connection);
                break;
            case BD_SOCKET_LOST:
                bdAddressMap::addrToString(connection->getAddressHandle(), addrStr, sizeof(addrStr));
                bdLogInfo("bdConnection/connectionstore", "Socket router reports socket lost. Disconnecting %s.", addrStr);
                toDisconnect.enqueue(&connection);
                break;
            default:
                break;
            }
        }
        else
        {
            reference_cast<bdLoopbackConnection, bdConnection>(bdConnectionRef(connection))->updateStatus();
            if (connection->getStatus() == bdConnection::BD_DISCONNECTED)
            {
                toDisconnect.enqueue(&connection);
            }
        }
    }
    while (!toDisconnect.isEmpty())
    {
        bdConnectionRef connection(toDisconnect.peek());
        bdAddrHandleRef  addr(connection->getAddressHandle());
        connection->close();
        if (!m_connectionMap.remove(bdAddrHandleRefWrapper(bdAddrHandleRef(addr))))
        {
            bdAddressMap::addrToString(addr, addrStr, 0x16uLL);
            bdLogWarn("bdConnection/connectionstor", "Failed to remove addr %s from connection map.", addrStr);
            success = false;
        }
        if (!m_socket->disconnect(addr))
        {
            bdAddressMap::addrToString(addr, addrStr, 0x16uLL);
            bdLogWarn("bdConnection/connectionstor", "Failed to disconnect secure association for addr %s .", addrStr);
            success = false;
        }
        toDisconnect.dequeue();
    }
    if (m_status == BD_CONNECTION_STORE_SHUTTING_DOWN && (!m_connectionMap.getSize() || m_shutdownTimer.getElapsedTimeInSeconds() > m_config.m_maxShutdownDuration))
    {
        success = startShutdown(BD_CONNECTION_STORE_SHUTDOWN_IMMEDIATE);
    }
    return success;
}

bdUInt bdConnectionStore::flush(bdConnection& connection)
{
    bdUByte8 data[1256];

    bdUInt size = connection.getDataToSend(data, sizeof(data));
    if (size)
    {
        bdAddrHandleRef addrHandle(connection.getAddressHandle());
        switch (m_socket->sendTo(bdAddrHandleRef(addrHandle), data, size))
        {
        case BD_NET_ADDRESS_INVALID:
            bdLogInfo("bdConnection/connectionstore", "Invalid address. Closing connection.");
            connection.close();
            break;
        case BD_NET_SUBSYTEM_ERROR:
            bdLogWarn("bdConnection/connectionstore", "net subsystem error!");
            break;
        case BD_NET_WOULD_BLOCK:
            bdLogWarn("bdConnection/connectionstore", "would block.");
            break;
        case BD_NET_ERROR:
            bdLogWarn("bdConnection/connectionstore", "unknown error.");
            break;
        default:
            break;
        }
    }
    return size;
}

bdBool bdConnectionStore::startShutdown(const bdConnectionStore::bdConnectionStoreShutdownType shutdownType)
{
    bdHashMap<bdAddrHandleRefWrapper,bdReference<bdConnection>,bdAddrHandleRefWrapper>::Iterator it;
    if (!m_status)
    {
        bdLogWarn("bdConnection/connectionstore", "Class is uninitialized, nothing for shutdown to do.");
        return false;
    }

    if (shutdownType == BD_CONNECTION_STORE_SHUTDOWN_IMMEDIATE)
    {
        for (it = m_connectionMap.getIterator(); it; m_connectionMap.next(it))
        {
            bdConnectionRef conn(m_connectionMap.getValue(it));
            conn->disconnect();
            conn->close();
        }
        m_connectionMap.clear();
        m_dispatcher.reset();
        m_socket = NULL;
        m_status = BD_CONNECTION_STORE_UNINITIALIZED;
        m_shutdownTimer.reset();
        m_addrMap = NULL;
        m_config.reset();
        return true;
    }
    else if (shutdownType == BD_CONNECTION_STORE_SHUTDOWN_ELEGANT)
    {
        if (m_status == BD_CONNECTION_STORE_INITIALIZED || m_status == BD_CONNECTION_STORE_ERROR)
        {
            for (it = m_connectionMap.getIterator(); it; m_connectionMap.next(it))
            {
                bdConnectionRef conn(m_connectionMap.getValue(it));
                conn->disconnect();
            }
            m_status = BD_CONNECTION_STORE_SHUTTING_DOWN;
            m_shutdownTimer.start();
            return true;
        }
        else if (m_status == BD_CONNECTION_STORE_SHUTTING_DOWN)
        {
            bdLogWarn("bdConnection/connectionstore", "Already shutting down; Shutdown call ignored.");
            return false;
        }
    }
    return false;
}

bdBool bdConnectionStore::receiveAll()
{
    bdInt received;
    bdUByte8 data[1288];
    bdBool receivedData = true;
    bdBool success = true;

    if (m_status != BD_CONNECTION_STORE_INITIALIZED && m_status != BD_CONNECTION_STORE_SHUTTING_DOWN)
    {
        bdLogWarn("bdConnection/connectionstore", "Cannot call receiveAll unless class is initialized.");
        return false;
    }

    for (bdUInt iterationCount = 0; receivedData && iterationCount < m_config.m_maxReceiveIterations; ++iterationCount)
    {
        bdAddrHandleRef address;
        received = m_socket->receiveFrom(address, data, sizeof(data));
        receivedData = received > 0;
        if (received < 0)
        {
            receivedData = 0;
            switch (received)
            {
            case -1:
                bdLogWarn("bdConnection/connectionstore", "Encountered error: %i while receiving from the socket router ", received);
                success = false;
                break;
            case -5:
                success = true;
                receivedData = true;
                break;
            case -2:
                success = true;
                break;
            case 1:
                bdLogError("bdConnection/connectionstore", "These cannot occur");
                success = false;
                break;
            default:
                break;
            }
            continue;
        }
        bdConnectionRef receivedConnection;
        if (address.isNull())
        {
            bdLogError("bdConnection/connectionstore", "Data & null addr handle ref received from socket route receiveFrom, ignoring.");
            continue;
        }
        bdBool connRecv = false;
        if (m_connectionMap.get(bdAddrHandleRefWrapper(bdAddrHandleRef(address)), receivedConnection))
        {
            if (receivedConnection.notNull())
            {
                connRecv = receivedConnection->receive(data, received);
            }
        }
        if (connRecv)
        {
            if (!receivedConnection->getAddressHandle()->getRealAddr().getAddress().isValid())
            {
                receivedConnection->setAddressHandle(&address);
            }
        }
        else if (m_status != BD_CONNECTION_STORE_SHUTTING_DOWN)
        {
            bdUnicastConnection newConnection(m_addrMap);
            newConnection.setAddressHandle(&address);
            if (!newConnection.receive(data, received))
            {
                success = false;
                continue;
            }
            if (newConnection.getStatus() != bdConnection::BD_CONNECTED)
            {
                flush(newConnection);
                continue;
            }
            bdConnectionRef establishedConnection(new bdUnicastConnection(m_addrMap));
            establishedConnection->setAddressHandle(&address);
            if (establishedConnection->receive(data, received) && establishedConnection->getStatus() == bdConnection::BD_CONNECTED)
            {
                if (m_connectionMap.put(bdAddrHandleRefWrapper(bdAddrHandleRef(address)), establishedConnection))
                {
                    bdLogInfo("bdConnection/connectionstore", "New incoming connection created.");
                }
            }
        }
    }
    return success;
}

void bdConnectionStore::dispatchAll()
{
    bdHashMap<bdAddrHandleRefWrapper, bdReference<bdConnection>, bdAddrHandleRefWrapper>::Iterator it;

    bdArray<bdConnectionRef> connections(m_connectionMap.getSize());
    for (it = m_connectionMap.getIterator(); it; m_connectionMap.next(it))
    {
        connections.pushBack(m_connectionMap.getValue(it));
    }
    for (bdUInt i = 0; i < connections.getSize(); ++i)
    {
        bdConnectionRef connection(connections[i]);
        m_dispatcher.process(&connection);
    }
}

void bdConnectionStore::registerDispatchInterceptor(bdDispatchInterceptor* const interceptor)
{
    m_dispatcher.registerInterceptor(interceptor);
}

void bdConnectionStore::unregisterDispatchInterceptor(bdDispatchInterceptor* const interceptor)
{
    m_dispatcher.unregisterInterceptor(interceptor);
}

bdConnectionStore::bdConnectionStoreStatus bdConnectionStore::getStatus() const
{
    return m_status;
}
