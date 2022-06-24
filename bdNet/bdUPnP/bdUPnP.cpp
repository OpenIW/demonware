// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdNet/bdNet.h"

bdUPnP::bdUPnP()
    : m_state(BD_UPNP_UNINITIALISED), m_gamePort(0), m_localAddrs(NULL), m_config(), m_discoverySocket(), m_bytesReceived(0), m_device(), m_cachedDevice(), m_discoveryTimer(), m_numDiscoveryAttempts(0),
    m_shutdownRequested(false)
{
}

bdUPnP::~bdUPnP()
{
}

bdBool bdUPnP::init(bdPort gamePort, bdArray<bdInetAddr>* localAddrs, bdUPnPConfig config)
{
    if (config.m_disabled)
    {
        bdLogInfo("bdnet/upnp", "UPnP disabled.");
        m_state = BD_UPNP_FINISHED;
        return true;
    }
    if (m_state)
    {
        bdLogWarn("bdnet/upnp", "Cannot initialize class unless it is in the uninitialized state (%d).", m_state);
        return false;
    }
    if (localAddrs->isEmpty())
    {
        bdLogWarn("bdnet/upnp", "Cannot start class unless at least one local common addr is supplied..");
        return false;
    }
    m_localAddrs = localAddrs;
    m_gamePort = gamePort;
    m_config = config;
    m_config.sanityCheckConfig();
    if (!m_config.m_gatewayAddr.isValid())
    {
        bdLogWarn("bdnet/upnp", "Starting UPnP without setting a gateway address in the config. Will use first UPnP device found");
    }
    m_state = BD_UPNP_INITIALISED;
    return true;
}

void bdUPnP::pump()
{
    if (m_shutdownRequested)
    {
        m_shutdownRequested = false;
        m_state = BD_UPNP_SHUTTING_DOWN;
    }
    switch (m_state)
    {
    case BD_UPNP_UNINITIALISED:
        bdLogWarn("bdnet/upnp", "Pumping an uninitialized device has no effect.");
        break;

    case BD_UPNP_INITIALISED:
        if (!setupDiscoverySocket())
        {
            bdLogWarn("bdnet/upnp", "UPnP failed to setup discovery socket");
            m_state = BD_UPNP_FAILED;
            break;
        }
        if (!startDiscovery())
        {
            bdLogWarn("bdnet/upnp", "UPnP failed to start discovery");
            m_state = BD_UPNP_FAILED;
            break;
        }
        m_bytesReceived = 0;
        bdMemset(m_readBuffer, 0, sizeof(m_readBuffer));
        m_discoveryTimer.reset();
        m_discoveryTimer.start();
        ++m_numDiscoveryAttempts;
        m_state = BD_UPNP_DISCOVERING_DEVICES;
        break;

    case BD_UPNP_DISCOVERING_DEVICES:
    {
        if (m_discoveryTimer.getElapsedTimeInSeconds() < m_config.m_discoveryTimeout)
        {
            bdUPnPDevice discoveredDevice;
            bdBool ok = checkForDiscoveredDevices(&discoveredDevice);
            if (discoveredDevice.getDeviceState() == bdUPnPDevice::BD_UPNP_DEVICE_INITIALISED && ok)
            {
                if (!m_config.m_gatewayAddr.isValid())
                {
                    bdLogInfo("bdnet/upnp", "UPnP capable device discovered and no gateway set - using it. Finished Discovery.");
                    bdMemcpy(&m_device, &discoveredDevice, sizeof(m_device));
                    m_state = BD_UPNP_CONFIGURING_DEVICE;
                    break;
                }

                if (discoveredDevice.getDeviceAddr().getAddress() == m_config.m_gatewayAddr)
                {
                    bdLogInfo("bdnet/upnp", "UPnP capable device discovered which matches gateway. Finished Discovery.");
                    bdMemcpy(&m_device, &discoveredDevice, sizeof(m_device));
                    m_state = BD_UPNP_CONFIGURING_DEVICE;
                }
                else if (m_cachedDevice.getDeviceState())
                {
                    discoveredDevice.startShutdown(bdUPnPDevice::BD_UPNP_DEVICE_SHUTDOWN_IMMEDIATE);
                    bdLogInfo("bdnet/upnp", "UPnP capable device discovered which does not match gateway. Ignoring.");
                }
                else
                {
                    bdLogInfo("bdnet/upnp", "UPnP capable device discovered which does not match gateway. Setting as cached device.");
                    bdMemcpy(&m_cachedDevice, &discoveredDevice, sizeof(m_cachedDevice));
                }
            }
            else if (!ok)
            {
                bdLogWarn("bdnet/upnp", "UPnP error discovering devices");
                m_state = BD_UPNP_FAILED;
            }
        }
        else if (m_cachedDevice.getDeviceState() == bdUPnPDevice::BD_UPNP_DEVICE_INITIALISED)
        {
            bdLogInfo("bdnet/upnp", "Device discovery timed out after %fs. Using cached device", m_discoveryTimer.getElapsedTimeInSeconds());
            bdMemcpy(&m_device, &m_cachedDevice, sizeof(m_device));
            m_state = BD_UPNP_DISCOVERED_DEVICE;
        }
        else if (m_discoverySocket.close())
        {
            if (m_numDiscoveryAttempts > m_config.m_discoveryRetries)
            {
                bdLogInfo("bdnet/upnp", "Discovery timed out. Max retries reached. Stopping");
                m_state = BD_UPNP_FINISHED;
                break;
            }
            bdLogInfo("bdnet/upnp", "Discovery timed out. Retrying.");
            m_state = BD_UPNP_INITIALISED;
        }
        else
        {
            bdLogWarn("bdnet/upnp", "UPnP failed to close discovery socket");
            m_state = BD_UPNP_FAILED;
        }
        break;

    case BD_UPNP_DISCOVERED_DEVICE:
        if (!m_discoverySocket.close())
        {
            bdLogWarn("bdnet/upnp", "UPnP unable to close discovery socket");
            m_state = BD_UPNP_FAILED;
            break;
        }
        m_state = BD_UPNP_CONFIGURING_DEVICE;
        break;

    case BD_UPNP_CONFIGURING_DEVICE:
        m_device.pump();
        if (m_device.getDeviceState() == bdUPnPDevice::BD_UPNP_DEVICE_FINISHED)
        {
            m_state = BD_UPNP_FINISHED;
        }
        else if (m_device.getDeviceState() == bdUPnPDevice::BD_UPNP_DEVICE_FAILED || m_device.getDeviceState() == bdUPnPDevice::BD_UPNP_DEVICE_UNINITIALISED)
        {
            bdLogWarn("bdnet/upnp", "UPnP failed to configure device");
            m_state = BD_UPNP_FAILED;
        }
        break;

    case BD_UPNP_SHUTTING_DOWN:
        m_device.pump();
        if (m_device.getDeviceState() != bdUPnPDevice::BD_UPNP_DEVICE_SHUTDOWN_ELEGANT)
        {
            cleanup();
            break;
        }
        startShutdown(bdUPnPDevice::BD_UPNP_DEVICE_SHUTDOWN_IMMEDIATE);
        break;
    }

    case BD_UPNP_FINISHED:
        return;

    case BD_UPNP_FAILED:
        m_discoverySocket.close();
        break;

    default:
        bdLogWarn("bdnet/upnp", "Pumping UPnP In Unknown State");
        m_state = BD_UPNP_FAILED;
        break;
    }
}

void bdUPnP::cleanup()
{
    m_state = BD_UPNP_UNINITIALISED;
    m_shutdownRequested = false;
    m_gamePort = 0;
    m_localAddrs = NULL;
    m_config.reset();
    m_discoverySocket.close();
    m_bytesReceived = 0;
    m_device = bdUPnPDevice();
    m_cachedDevice = bdUPnPDevice();
    m_discoveryTimer.reset();
}

void bdUPnP::startShutdown(bdUPnPDevice::bdUPnPDeviceShutdownType shutdownType)
{
    if (m_state)
    {
        if (shutdownType == bdUPnPDevice::BD_UPNP_DEVICE_SHUTDOWN_IMMEDIATE)
        {
            m_device.startShutdown(bdUPnPDevice::BD_UPNP_DEVICE_SHUTDOWN_IMMEDIATE);
            cleanup();
        }
        else if (shutdownType == bdUPnPDevice::BD_UPNP_DEVICE_SHUTDOWN_ELEGANT && m_state != BD_UPNP_SHUTTING_DOWN)
        {
            m_device.startShutdown(bdUPnPDevice::BD_UPNP_DEVICE_SHUTDOWN_ELEGANT);
            m_shutdownRequested = 1;
        }
    }
}

bdBool bdUPnP::startDiscovery()
{
    bdInt discLength;
    bdByte8 messageBuffer[136];
    bdBool success = true;

    for (bdUInt i = 0; i < 2; ++i)
    {
        discLength = bdSnprintf(
            messageBuffer,
            sizeof(messageBuffer),
            "M-SEARCH * HTTP/1.1\r\n"
            "MX: 1\r\n"
            "HOST: 239.255.255.250:1900\r\n"
            "MAN: \"ssdp:discover\"\r\n"
            "ST: urn:schemas-upnp-org:service:%s\r\n"
            "\r\n",
            i ? "WANIPConnection:1" : "WANPPPConnection:1"
        );
        bdAddr discoveryAddr("239.255.255.250:1900");
        if (m_discoverySocket.sendTo(discoveryAddr, messageBuffer, discLength) != discLength)
        {
            bdLogError("bdnet/upnp", "Failed to send discovery packet");
            success = false;
        }
    }
    return success;
}

bdBool bdUPnP::setupDiscoverySocket()
{
    if (!m_discoverySocket.create(false, true))
    {
        bdLogError("bdnet/upnp", "Failed to create discovery socket.");
        return false;
    }
    return true;
}

bdBool bdUPnP::handleDiscoveryResponse(bdUPnPDevice* discoveredDevice)
{
    bdNChar8 addressBuffer[22];
    bdAddr deviceAddr;
    bdByte8* fetchLoc = NULL;
    bdUInt fetchLen = 0;

    if (!extractDeviceInfo(deviceAddr, &fetchLoc, fetchLen))
    {
        return false;
    }
    deviceAddr.toString(addressBuffer, sizeof(addressBuffer));
    bdLogInfo("bdnet/upnp", "Discovered Device at address %s", addressBuffer);
    return discoveredDevice->init(m_localAddrs, fetchLoc, fetchLen, &deviceAddr.getAddress(), deviceAddr.getPort(), bdUPnPConfig(m_config));
}

bdBool bdUPnP::checkForDiscoveredDevices(bdUPnPDevice* discoveredDevice)
{
    bdAddr incomingAddress;
    bdInt received = m_discoverySocket.receiveFrom(incomingAddress, m_readBuffer, sizeof(m_readBuffer));
    if (received <= 0 || received > 1024)
    {
        if (received == -2)
        {
            return true;
        }
        return false;
    }
    m_bytesReceived = received;
    m_readBuffer[m_bytesReceived] = 0;
    return handleDiscoveryResponse(discoveredDevice);
}

bdBool bdUPnP::extractDeviceInfo(bdAddr& deviceAddr, bdByte8** fetchLoc, bdUInt& fetchLen)
{
    bdNChar8* lineEnd;
    bdUInt addrStringLength;
    bdNChar8* slashLoc;
    bdNChar8* httpString;
    bdByte8 addrString[22];

    httpString = bdStrstr(m_readBuffer, "http://");
    if (!httpString)
    {
        bdLogWarn("bdnet/upnp", "no http:// token found in discovery response");
        return false;
    }
    httpString += 7;
    slashLoc = bdStrstr(httpString, "/");
    if (!slashLoc)
    {
        bdLogWarn("bdnet/upnp", "No fetch path found after URL");
        return false;
    }
    addrStringLength = slashLoc - httpString;
    if (slashLoc != httpString && addrStringLength < sizeof(addrString))
    {
        bdStrlcpy(addrString, httpString, addrStringLength + 1);
        deviceAddr = bdAddr(addrString);
        lineEnd = bdStrchr(httpString, 13);
        if (!lineEnd)
        {
            bdLogWarn("bdnet/upnp", "No carriage return after fetch location");
            return false;
        }
        *fetchLoc = slashLoc;
        fetchLen = lineEnd - slashLoc;
        return true;
    }
    return false;
}

bdBool bdUPnP::getExternalAddr(bdAddr& externalAddr)
{
    externalAddr = m_device.getExternalAddr();

    if (externalAddr.getAddress().isValid())
    {
        return true;
    }
    return false;
}


const bdBool bdUPnP::isRunning() const
{
    return m_state != BD_UPNP_FINISHED && m_state != BD_UPNP_FAILED && m_state;
}

const bdUPnPDevice::bdUPnPPortStatus bdUPnP::getPortStatus() const
{
    if (m_device.getDeviceState() == bdUPnPDevice::BD_UPNP_DEVICE_FINISHED)
    {
        return m_device.getPortStatus();
    }
    return bdUPnPDevice::BD_UPNP_PORT_STATUS_UNKNOWN;
}

const bdUPnP::bdUPnPState bdUPnP::getState() const
{
    return m_state;
}
