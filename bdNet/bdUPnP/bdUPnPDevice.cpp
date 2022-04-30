// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdNet/bdNet.h"

bdUPnPDevice::bdUPnPDevice()
    : m_state(BD_UPNP_DEVICE_UNINITIALISED), m_shutdownRequested(false), m_portStatus(BD_UPNP_PORT_STATUS_UNKNOWN), m_deviceAddr(), m_localAddrs(NULL), m_localConnectedAddr(),
    m_streamSocket(), m_isIp(true), m_config(), m_gamePort(0), m_externalDeviceAddr(), m_requestSize(0), m_bytesReceived(0), m_connectTimer(), m_responseTimer()
{
}

bdUPnPDevice::~bdUPnPDevice()
{
}

bdBool bdUPnPDevice::init(bdArray<bdInetAddr>* localAddrs, const bdUByte8* const fetchLocation, const bdInt fetchLen, bdInetAddr* deviceAddr, const bdUInt16 port, const bdUPnPConfig config)
{
    if (m_state)
    {
        bdLogWarn("bdnet/upnpdevice", "Cannot initialize class unless it is in the uninitialized state.");
        return false;
    }
    bdMemcpy(&m_deviceAddr, deviceAddr, sizeof(m_deviceAddr));
    m_gamePort = port;
    bdMemcpy(&m_config, &config, sizeof(m_config));
    m_localAddrs = localAddrs;
    bdSnprintf(m_fetchLocation, fetchLen + 1, reinterpret_cast<const char* const>(fetchLocation));
    m_state = BD_UPNP_DEVICE_INITIALISED;
    m_portStatus = BD_UPNP_PORT_STATUS_UNKNOWN;
    m_shutdownRequested = false;
    if (m_streamSocket.isConnected())
    {
        m_streamSocket.close();
    }
    return true;
}

void bdUPnPDevice::pump()
{
    bdBool ok;
    bdInt32 errorCode;
    bdBool isComplete;
    bdBool mappingExists;
    bdBool mappingIsMine;

    if (m_shutdownRequested)
    {
        m_shutdownRequested = false;
        m_state = BD_UPNP_DEVICE_SHUTTING_DOWN;
    }
    switch (m_state)
    {
    case BD_UPNP_DEVICE_UNINITIALISED:
        bdLogWarn("bdnet/upnpdevice", "Pumping an uninitialized device has no effect.");
        break;

    case BD_UPNP_DEVICE_INITIALISED:
        prepareGetDescriptionRequest();
        if (!connectToDevice())
        {
            bdLogWarn("bdnet/upnpdevice", "Failed to connect to device.");
            m_state = BD_UPNP_DEVICE_FAILED;
        }
        m_state = BD_UPNP_DEVICE_SENDING_DESCRIPTION_REQ;
        break;

    case BD_UPNP_DEVICE_SENDING_DESCRIPTION_REQ:
        isComplete = false;
        ok = pumpSendRequest(&isComplete);
        if (!ok)
        {
            bdLogWarn("bdnet/upnpdevice", "Failed to send device description request.");
            m_state = BD_UPNP_DEVICE_FAILED;
            break;
        }
        if (!isComplete)
        {
            break;
        }

        if (setLocalConnectedAddr())
        {
            m_state = BD_UPNP_DEVICE_GETTING_DESCRIPTION;
        }
        else
        {
            bdLogWarn("bdnet/upnpdevice", "Failed to set the local addr.");
            m_state = BD_UPNP_DEVICE_FAILED;
        }
        break;

    case BD_UPNP_DEVICE_GETTING_DESCRIPTION:
        if (pumpReceive())
        {
            break;
        }

        if (parseDescriptionResponse())
        {
            m_state = BD_UPNP_DEVICE_GOT_DESCRIPTION;
        }
        else
        {
            bdLogWarn("bdnet/upnpdevice", "Failed to parse the device description.");
            m_state = BD_UPNP_DEVICE_FAILED;
        }
        break;

    case BD_UPNP_DEVICE_GOT_DESCRIPTION:
        bdLogInfo("bdnet/upnpdevice", "Successfully received the device description");
        prepareGetExternalIPRequest();
        if (connectToDevice())
        {
            m_state = BD_UPNP_DEVICE_SENDING_IP_REQ;
        }
        else
        {
            bdLogWarn("bdnet/upnpdevice", "Failed to connect to device.");
            m_state = BD_UPNP_DEVICE_FAILED;
        }
        break;

    case BD_UPNP_DEVICE_SENDING_IP_REQ:
        isComplete = false;
        ok = pumpSendRequest(&isComplete);
        if (isComplete && ok)
        {
            m_state = BD_UPNP_DEVICE_DISCOVERING_IP;
        }
        else if (!ok)
        {
            bdLogWarn("bdnet/upnpdevice", "Failed to send the exernal IP request.");
            m_state = BD_UPNP_DEVICE_FAILED;
        }
        break;

    case BD_UPNP_DEVICE_DISCOVERING_IP:
        if (pumpReceive())
        {
            break;
        }

        if (parseGetExternalIPResponse())
        {
            m_state = BD_UPNP_DEVICE_DISCOVERED_IP;
        }
        else
        {
            bdLogWarn("bdnet/upnpdevice", "Failed to parse the external IP response.");
            m_state = BD_UPNP_DEVICE_FAILED;
        }
        break;

    case BD_UPNP_DEVICE_DISCOVERED_IP:
        if (m_config.m_runMode)
        {
            bdLogInfo("bdnet/upnpdevice", "UPnP device configuration complete");
            m_state = BD_UPNP_DEVICE_FINISHED;
            break;
        }
        prepareGetMappingRequest();
        if (connectToDevice())
        {
            m_state = BD_UPNP_DEVICE_SENDING_PORT_QUERY_REQ;
        }
        break;

    case BD_UPNP_DEVICE_SENDING_PORT_QUERY_REQ:
        isComplete = false;
        ok = pumpSendRequest(&isComplete);
        if (isComplete && ok)
        {
            m_state = BD_UPNP_DEVICE_DISCOVERING_PORT_MAPPING;
        }
        else if (!ok)
        {
            bdLogWarn("bdnet/upnpdevice", "Failed to send port query request.");
            m_state = BD_UPNP_DEVICE_FAILED;
        }
        break;

    case BD_UPNP_DEVICE_DISCOVERING_PORT_MAPPING:
        isComplete = false;
        mappingExists = false;
        mappingIsMine = false;

        if (pumpReceive())
        {
            break;
        }

        if (parseGetMappingsResponse(&mappingExists, &mappingIsMine))
        {
            if (mappingExists && mappingIsMine)
            {
                m_portStatus = BD_UPNP_PORT_MAPPED;
            }
            else if (!mappingExists || mappingIsMine)
            {
                m_portStatus = BD_UPNP_PORT_NOT_MAPPED;
            }
            else
            {
                m_portStatus = BD_UPNP_PORT_COLLISION_FOUND;
            }
            m_state = BD_UPNP_DEVICE_DISCOVERED_PORT_MAPPING;
        }
        else
        {
            bdLogWarn("bdnet/upnpdevice", "Failed to parse port query response");
            m_state = BD_UPNP_DEVICE_FAILED;
        }
        break;

    case BD_UPNP_DEVICE_DISCOVERED_PORT_MAPPING:
        switch (m_portStatus)
        {
        case BD_UPNP_PORT_MAPPED:
            bdLogInfo("bdnet/upnpdevice", "Usable Port Mapping found. UPnP device configuration complete.");
            m_state = BD_UPNP_DEVICE_FINISHED;
            break;
        case BD_UPNP_PORT_COLLISION_FOUND:
            bdLogInfo("bdnet/upnpdevice", "Port Collision Detected. UPnP device configuration complete");
            m_state = BD_UPNP_DEVICE_FINISHED;
            break;
        case BD_UPNP_PORT_NOT_MAPPED:
            prepareCreatePortMappingRequest();
            if (connectToDevice())
            {
                bdLogInfo("bdnet/upnpdevice", "No port mappings found. Creating one on port %u.", m_gamePort);
                m_state = BD_UPNP_DEVICE_SENDING_MAPPING_REQ;
            }
            else
            {
                bdLogWarn("bdnet/upnpdevice", "Failed to connect to device.");
                m_state = BD_UPNP_DEVICE_FAILED;
            }
            break;
        default:
            bdLogWarn("bdnet/upnpdevice", "Unexpected port status");
            m_state = BD_UPNP_DEVICE_FAILED;
            break;
        }
        break;

   case BD_UPNP_DEVICE_SENDING_MAPPING_REQ:
        isComplete = false;
        ok = pumpSendRequest(&isComplete);
        if (isComplete && ok)
        {
            m_state = BD_UPNP_DEVICE_MAPPING_PORT;
        }
        else if (!ok)
        {
            bdLogWarn("bdnet/upnpdevice", "Failed to send port mapping request.");
            m_state = BD_UPNP_DEVICE_FAILED;
        }
        break;

    case BD_UPNP_DEVICE_MAPPING_PORT:
        if (pumpReceive())
        {
            break;
        }
        
        if (confirmHttpSuccess())
        {
            m_portStatus = BD_UPNP_PORT_MAPPED;
            m_state = BD_UPNP_DEVICE_MAPPED_PORT;
        }
        else
        {
            bdLogWarn("bdnet/upnpdevice", "Failed to map port.");
            m_state = BD_UPNP_DEVICE_FAILED;
        }
        break;

    case BD_UPNP_DEVICE_MAPPED_PORT:
        if (m_portStatus == BD_UPNP_PORT_MAPPED)
        {
            bdLogInfo("bdnet/upnpdevice", "Port successfully mapped. UPnP device configuration complete");
            m_state = BD_UPNP_DEVICE_FINISHED;
        }
        else
        {
            bdLogWarn("bdnet/upnpdevice", "Unexpected port status");
            m_state = BD_UPNP_DEVICE_FAILED;
        }
        break;

    case BD_UPNP_DEVICE_SENDING_UNMAPPING_REQ:
        isComplete = false;
        ok = pumpSendRequest(&isComplete);
        if (isComplete && ok)
        {
            m_state = BD_UPNP_DEVICE_UNMAPPING_PORT;
        }
        else if (!ok)
        {
            bdLogWarn("bdnet/upnpdevice", "Failed to send port unmapping request.");
            m_state = BD_UPNP_DEVICE_FAILED;
        }
        break;

    case BD_UPNP_DEVICE_UNMAPPING_PORT:
        if (pumpReceive())
        {
            break;
        }

        if (confirmHttpSuccess())
        {
            bdLogInfo("bdnet/upnpdevice", "Port unmapping successful on port %u", m_gamePort);
            m_portStatus = BD_UPNP_PORT_NOT_MAPPED;
            m_state = BD_UPNP_DEVICE_UNMAPPED_PORT;
        }
        else
        {
            bdLogWarn("bdnet/upnpdevice", "Failed to unmap port.");
            m_state = BD_UPNP_DEVICE_FAILED;
        }
        break;

    case BD_UPNP_DEVICE_UNMAPPED_PORT:
        if (m_portStatus == BD_UPNP_PORT_NOT_MAPPED)
        {
            m_shutdownType = BD_UPNP_DEVICE_SHUTDOWN_IMMEDIATE;
            m_state = BD_UPNP_DEVICE_SHUTTING_DOWN;
        }
        else
        {
            bdLogWarn("bdnet/upnpdevice", "Unexpected port status");
            m_state = BD_UPNP_DEVICE_FAILED;
        }
        break;

    case BD_UPNP_DEVICE_SHUTTING_DOWN:
        m_streamSocket.close();
        if (m_shutdownType || m_portStatus != BD_UPNP_PORT_MAPPED)
        {
            cleanup();
            break;
        }
        preparePortUnMappingRequest();
        if (connectToDevice())
        {
            m_state = BD_UPNP_DEVICE_SENDING_UNMAPPING_REQ;
        }
        else
        {
            bdLogWarn("bdnet/upnpdevice", "Failed to connect to device.");
            m_state = BD_UPNP_DEVICE_FAILED;
        }
        break;

    case BD_UPNP_DEVICE_FINISHED:
        return;

    case BD_UPNP_DEVICE_FAILED:
        if (m_streamSocket.isConnected())
        {
            m_streamSocket.close();
        }
        break;

    default:
        bdLogWarn("bdnet/upnpdevice", "Pumping Device in Unknown State");
        m_state = BD_UPNP_DEVICE_FAILED;
        break;
    }
}

void bdUPnPDevice::cleanup()
{
    m_state = BD_UPNP_DEVICE_UNINITIALISED;
    m_shutdownRequested = 0;
    m_portStatus = BD_UPNP_PORT_STATUS_UNKNOWN;
    bdMemcpy(&m_deviceAddr, &bdAddr(), sizeof(m_deviceAddr));
    m_localAddrs = NULL;
    bdMemcpy(&m_localConnectedAddr, &bdInetAddr(), sizeof(m_localConnectedAddr));
    m_streamSocket.close();
    bdMemcpy(&m_config, &bdUPnPConfig(), sizeof(m_config));
    m_gamePort = 0;
    bdMemcpy(&m_externalDeviceAddr, &bdInetAddr(), sizeof(m_externalDeviceAddr));
    m_requestSize = 0;
    m_bytesReceived = 0;
}

void bdUPnPDevice::startShutdown(bdUPnPDeviceShutdownType shutdownType)
{
    if (m_state)
    {
        if (shutdownType == BD_UPNP_DEVICE_SHUTDOWN_IMMEDIATE)
        {
            bdUPnPDevice::cleanup();
        }
        else if (shutdownType == BD_UPNP_DEVICE_SHUTDOWN_ELEGANT
            && m_state != BD_UPNP_DEVICE_SHUTTING_DOWN
            && m_state != BD_UPNP_DEVICE_SENDING_UNMAPPING_REQ
            && m_state != BD_UPNP_DEVICE_UNMAPPING_PORT
            && m_state != BD_UPNP_DEVICE_UNMAPPED_PORT)
        {
            m_shutdownType = BD_UPNP_DEVICE_SHUTDOWN_ELEGANT;
            m_shutdownRequested = 1;
        }
    }
}

void bdUPnPDevice::prepareGetDescriptionRequest()
{
    bdNChar8 deviceAddrBuffer[22];

    bdUInt totalRequestSize = m_deviceAddr.toString(deviceAddrBuffer, sizeof(deviceAddrBuffer))
        + bdStrlen(m_fetchLocation)
        + bdStrlen("GET %s HTTP/1.1\r\nConnection: close\r\nHost: %s\r\n\r\n");
    bdAssert(totalRequestSize < BD_UPNP_MAX_DEVICE_REQUEST_SIZE, "Request is too large for buffer");
    m_requestSize = bdSnprintf(m_requestBuffer, BD_UPNP_MAX_DEVICE_REQUEST_SIZE, "GET %s HTTP/1.1\r\nConnection: close\r\nHost: %s\r\n\r\n", m_fetchLocation, deviceAddrBuffer);
}

bdBool bdUPnPDevice::parseDescriptionResponse()
{
    bdByte8* URLBase = NULL;
    bdUInt URLBaseLen = 0;
    bdBool isIP = false;
    bdByte8* controlURL = false;
    bdUInt controlURLLen = 0;
    bdBool ok = confirmHttpSuccess();

    ok = ok == extractURLBase(&URLBase, &URLBaseLen);
    ok = ok == extractServiceType(&isIP);
    ok = ok == extractControlURL(&controlURL, &controlURLLen);

    if (!ok)
    {
        return false;
    }
    m_isIp = isIP;
    if (URLBaseLen && controlURLLen + URLBaseLen < 128)
    {
        if (URLBase[URLBaseLen - 1] == '/' && *controlURL == '/')
        {
            ++controlURL;
            --controlURLLen;
        }
        bdStrlcpy(m_controlURL, URLBase, URLBaseLen + 1);
        bdStrlcpy(&m_controlURL[URLBaseLen], controlURL, controlURLLen + 1);
    }
    else if (controlURLLen && controlURLLen < 128)
    {
        bdStrlcpy(m_controlURL, controlURL, controlURLLen + 1);
    }
    else
    {
        return false;
    }
    return ok;
}

bdBool bdUPnPDevice::connectToDevice()
{
    bdSocketStatusCode connectStatus;

    if (!m_streamSocket.create(false))
    {
        return false;
    }
    if (!m_deviceAddr.getPort())
    {
        m_deviceAddr.setPort(80);
    }
    connectStatus = m_streamSocket.connect(&bdAddr(&m_deviceAddr));
    if (connectStatus == 1 || connectStatus == -2)
    {
        m_connectTimer.reset();
        m_connectTimer.start();
        return true;
    }
    return false;
}

bdBool bdUPnPDevice::pumpSendRequest(bdBool* isComplete)
{
    *isComplete = false;
    if (m_streamSocket.isConnected())
    {
        bdUInt requestLen = bdStrlen(m_requestBuffer);
        bdAssert(requestLen == m_requestSize, "Corrupted Buffer");
        bdInt sendResult = m_streamSocket.send(m_requestBuffer, m_requestSize);
        if (sendResult <= 0 && sendResult != 2)
        {
            bdLogWarn("bdnet/upnpdevice", "Error sending to UPnP Device");
            m_streamSocket.close();
            return false;
        }
        else
        {
            setupReceive();
            *isComplete = true;
        }
    }
    else if (m_connectTimer.getElapsedTimeInSeconds() > m_config.m_connectTimeout)
    {
        bdLogWarn("bdnet/upnpdevice", "Timed out while connecting to UPnP device");
        m_streamSocket.close();
        return false;
    }
    return true;
}

bdBool bdUPnPDevice::setLocalConnectedAddr()
{
    if (!m_localAddrs->isEmpty())
    {
        bdMemcpy(&m_localConnectedAddr, &m_localAddrs[0], sizeof(m_localConnectedAddr));
        return true;
    }
    return false;
}

bdBool bdUPnPDevice::pumpReceive()
{
    bdInt bytesRecieved;
    bdBool recieved = false;

    if (m_config.m_responseTimeout < m_responseTimer.getElapsedTimeInSeconds())
    {
        bdLogWarn("bdnet/upnpdevice", "Timed out while getting response from device");
    }
    else
    {
        bytesRecieved = m_streamSocket.recv(&m_readBuffer[m_bytesReceived], 0x1800 - m_bytesReceived);
        if (bytesRecieved <= 0)
        {
            if (bytesRecieved = -2)
            {
                recieved = true;
            }
            else if (bytesRecieved)
            {
                bdLogInfo("bdnet/upnpdevice", "Error recieving from socket %d", bytesRecieved);
            }
        }
        else
        {
            m_readBuffer[bytesRecieved + m_bytesReceived] = 0;
            m_bytesReceived += bytesRecieved;
            recieved = true;
        }
    }
    if (!recieved)
    {
        m_streamSocket.close();
    }
    return recieved;
}

void bdUPnPDevice::prepareGetExternalIPRequest()
{
    genUPnPCommand("GetExternalIPAddress", "\r\n");
}

bdBool bdUPnPDevice::parseGetExternalIPResponse()
{
    bdInetAddr externalAddress;
    bdBool ok = confirmHttpSuccess();
    if (ok)
    {
        ok = extractExternalAddress(&externalAddress);
    }
    if (ok)
    {
        bdMemcpy(&m_externalDeviceAddr, &externalAddress, sizeof(m_externalDeviceAddr));
    }
    return ok;
}

bdBool bdUPnPDevice::confirmHttpSuccess()
{
    return bdStrstr(m_readBuffer, "200 OK") != 0;
}

bdBool bdUPnPDevice::extractUPnPError(bdInt32* errorCode)
{
    return bdBool();
}

void bdUPnPDevice::selectNewExternalPort()
{
}

void bdUPnPDevice::preparePortUnMappingRequest()
{
}

void bdUPnPDevice::setupReceive()
{
}

void bdUPnPDevice::prepareCreatePortMappingRequest()
{
}

bdBool bdUPnPDevice::extractTag(const bdByte8* tag, bdByte8* buffer, bdByte8** value, bdUInt* valueLen)
{
    return bdBool();
}

bdBool bdUPnPDevice::checkForFinishedSoapResponse()
{
    return bdBool();
}

void bdUPnPDevice::genUPnPCommand(const bdByte8* const command, const bdByte8* const arguments)
{
}

bdBool bdUPnPDevice::extractURLBase(bdByte8** baseLoc, bdUInt* baseLen)
{
    return bdBool();
}

bdBool bdUPnPDevice::extractServiceType(bdBool* serviceType)
{
    return bdBool();
}

bdBool bdUPnPDevice::extractControlURL(bdByte8** controlLoc, bdUInt* controlLen)
{
    return bdBool();
}

bdBool bdUPnPDevice::extractExternalAddress(bdInetAddr* externalAddress)
{
    return bdBool();
}

void bdUPnPDevice::prepareGetMappingRequest()
{
}

bdBool bdUPnPDevice::parseGetMappingsResponse(bdBool* mappingExists, bdBool* mappingIsMine)
{
    return bdBool();
}

bdBool bdUPnPDevice::extractMappingOwner(bdInetAddr* owner, bdByte8* addrLoc, bdUInt addrLen)
{
    return bdBool();
}

const bdUPnPDevice::bdUPnPDeviceState bdUPnPDevice::getDeviceState() const
{
    return bdUPnPDeviceState();
}

const bdUPnPDevice::bdUPnPPortStatus bdUPnPDevice::getPortStatus() const
{
    return bdUPnPPortStatus();
}

const bdAddr* bdUPnPDevice::getExternalAddr() const
{
    return nullptr;
}

const bdAddr* bdUPnPDevice::getDeviceAddr() const
{
    return nullptr;
}
