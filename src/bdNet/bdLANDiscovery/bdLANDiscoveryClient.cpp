// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdNet/bdNet.h"

void bdLANDiscoveryClient::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdLANDiscoveryClient::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdLANDiscoveryClient::bdLANDiscoveryClient() : m_timeout(0.0f), m_timer(), m_socket(), m_listeners(), m_status(BD_IDLE)
{
}

bdLANDiscoveryClient::~bdLANDiscoveryClient()
{
    stop();
}

const bdLANDiscoveryClient::bdStatus bdLANDiscoveryClient::getStatus() const
{
    return m_status;
}

void bdLANDiscoveryClient::registerListener(bdLANDiscoveryListener* listener)
{
    m_listeners.pushBack(listener);
}

void bdLANDiscoveryClient::stop()
{
    m_socket.close();
    m_status = BD_IDLE;
}

void bdLANDiscoveryClient::unregisterListener(bdLANDiscoveryListener* listener)
{
    m_listeners.removeAllKeepOrder(listener);
}

bdBool bdLANDiscoveryClient::discover(const bdUInt titleID, const bdFloat32 timeout, const bdInetAddr& addr, const bdUInt16 discoveryPort)
{
    if (m_status == BD_UPLOAD)
    {
        bdLogWarn("bdNet/discovery", "bdLANDiscoveryClient::discover, LAN discovery already running");
        return false;
    }
    if (!m_socket.create(false, true))
    {
        bdLogError("bdNet/discovery", "bdLANDiscoveryClient::discover, Could not create socket");
        return false;
    }
    bdSingleton<bdTrulyRandomImpl>::getInstance()->getRandomUByte8(m_nonce, sizeof(m_nonce));
    m_timeout = timeout;
    bdBitBuffer* bitBuffer = new bdBitBuffer(0, 0);
    bdUByte8 type = 27;
    bitBuffer->writeDataType(BD_BB_UNSIGNED_CHAR8_TYPE);
    bitBuffer->writeBits(&type, CHAR_BIT);
    bitBuffer->writeDataType(BD_BB_FULL_TYPE);
    bitBuffer->writeBits(m_nonce, CHAR_BIT * sizeof(m_nonce));
    bitBuffer->writeDataType(BD_BB_UNSIGNED_INTEGER32_TYPE);
    bitBuffer->writeBits(&titleID, CHAR_BIT * sizeof(titleID));
    if (m_socket.sendTo(bdAddr(addr, discoveryPort), bitBuffer->getData(), bitBuffer->getDataSize()) >= 0)
    {
        m_timer.start();
        m_status = BD_UPLOAD;
        bdLogInfo("bdNet/discovery", "Starting: will run for %f seconds.", timeout);
        return true;
    }
    else
    {
        m_status = BD_DOWNLOAD;
        bdLogError("bdNet/discovery", "bdLANDiscoveryClient::discover, Socket sendTo failed");
        return false;
    }
}

void bdLANDiscoveryClient::update()
{
    // unused
}
