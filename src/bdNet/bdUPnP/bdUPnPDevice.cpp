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

bdBool bdUPnPDevice::init(bdArray<bdInetAddr>* localAddrs, const bdByte8* const fetchLocation, const bdInt fetchLen, const bdInetAddr* deviceAddr, const bdUInt16 port, const bdUPnPConfig config)
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
        ok = pumpSendRequest(isComplete);
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
        ok = pumpSendRequest(isComplete);
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
        prepareGetMappingsRequest();
        if (connectToDevice())
        {
            m_state = BD_UPNP_DEVICE_SENDING_PORT_QUERY_REQ;
        }
        break;

    case BD_UPNP_DEVICE_SENDING_PORT_QUERY_REQ:
        isComplete = false;
        ok = pumpSendRequest(isComplete);
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
        ok = pumpSendRequest(isComplete);
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
        ok = pumpSendRequest(isComplete);
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

    ok = ok == extractURLBase(&URLBase, URLBaseLen);
    ok = ok == extractServiceType(isIP);
    ok = ok == extractControlURL(&controlURL, controlURLLen);

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
    connectStatus = m_streamSocket.connect(bdAddr(m_deviceAddr));
    if (connectStatus == 1 || connectStatus == -2)
    {
        m_connectTimer.reset();
        m_connectTimer.start();
        return true;
    }
    return false;
}

bdBool bdUPnPDevice::pumpSendRequest(bdBool& isComplete)
{
    isComplete = false;
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
            isComplete = true;
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
        bytesRecieved = m_streamSocket.recv(&m_readBuffer[m_bytesReceived], sizeof(m_readBuffer) - m_bytesReceived);
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
        ok = extractExternalAddress(externalAddress);
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

void bdUPnPDevice::preparePortUnMappingRequest()
{
    bdNChar8 buf[112];

    bdSnprintf(buf, sizeof(buf), "<NewRemoteHost></NewRemoteHost>\r\n<NewExternalPort>%d</NewExternalPort>\r\n<NewProtocol>UDP</NewProtocol>\r\n", m_gamePort);
    genUPnPCommand("DeletePortMapping", buf);
}

void bdUPnPDevice::setupReceive()
{
    m_bytesReceived = 0;
    bdMemset(m_readBuffer, 0, sizeof(m_readBuffer));
    m_responseTimer.reset();
    m_responseTimer.start();
}

void bdUPnPDevice::prepareCreatePortMappingRequest()
{
    bdByte8 addrString[22];
    bdByte8 portMapArguments[412];

    m_localConnectedAddr.toString(addrString, sizeof(addrString));
    bdSnprintf(
        portMapArguments,
        0x19CuLL,
        "      <NewRemoteHost></NewRemoteHost>\r\n"
        "      <NewExternalPort>%d</NewExternalPort>\r\n"
        "      <NewProtocol>UDP</NewProtocol>\r\n"
        "      <NewInternalPort>%d</NewInternalPort>\r\n"
        "      <NewInternalClient>%s</NewInternalClient>\r\n"
        "      <NewEnabled>1</NewEnabled>\r\n"
        "      <NewPortMappingDescription>DemonwarePortMapping</NewPortMappingDescription>\r\n"
        "      <NewLeaseDuration>0</NewLeaseDuration>\r\n",
        m_gamePort,
        m_gamePort,
        addrString);
    genUPnPCommand("AddPortMapping", portMapArguments);
}

bdBool bdUPnPDevice::extractTag(const bdByte8* tag, bdByte8* buffer, bdByte8** value, bdUInt& valueLen)
{
    bdNChar8* elementEnd;
    bdByte8* elementStart;

    *value = 0LL;
    valueLen = 0;
    elementStart = bdStrstr(buffer, tag);
    if (!elementStart)
    {
        return false;
    }
    elementStart = bdStrstr(elementStart, ">");
    if (!elementStart)
    {
        return false;
    }
    elementEnd = bdStrstr(++elementStart, "</");
    if (!elementEnd)
    {
        bdLogWarn("bdnet/upnpdevice", "Tag %s is improperly formatted");
        return false;
    }
    if (elementEnd != elementStart)
    {
        *value = elementStart;
        valueLen = elementEnd - elementStart;
        return true;
    }
    return false;
}

void bdUPnPDevice::genUPnPCommand(const bdByte8* const command, const bdByte8* const arguments)
{
    bdUInt actualSoapSize;
    bdByte8 numBuff[10];
    bdByte8 addressBuff[22];
    bdByte8 soapBuffer[682];
    bdUInt totalRequestSize;
    bdInt numLen;

    actualSoapSize = bdSnprintf(
        soapBuffer,
        sizeof(soapBuffer),
        "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schema"
        "s.xmlsoap.org/soap/encoding/\">\r\n"
        "<s:Body>\r\n"
        "<u:%s xmlns:u=\"urn:schemas-upnp-org:service:%s\">\r\n"
        "%s</u:%s>\r\n"
        "</s:Body>\r\n"
        "</s:Envelope>\r\n",
        command,
        m_isIp ? "WANIPConnection:1" : "WANPPPConnection:1",
        arguments,
        command);
    m_deviceAddr.toString(addressBuff, sizeof(addressBuff));
    numLen = bdSnprintf(numBuff, sizeof(numBuff), "%u", actualSoapSize);
    totalRequestSize = actualSoapSize + numLen + bdStrlen(command) + 
        (bdStrlen(m_isIp ? "WANIPConnection:1" : "WANPPPConnection:1") + bdStrlen(addressBuff) + bdStrlen(m_controlURL) + bdStrlen("POST %s HTTP/1.1\r\n"
            "HOST: %s\r\n"
            "SOAPACTION: \"urn:schemas-upnp-org:service:%s#%s\"\r\n"
            "CONTENT-TYPE: text/xml; charset=\"utf-8\"\r\n"
            "CONTENT-LENGTH: %d\r\n"
            "\r\n"
            "%s"));
    bdAssert(totalRequestSize < BD_UPNP_MAX_DEVICE_REQUEST_SIZE, "Request is too large for buffer");
    m_requestSize = bdSnprintf(
        m_requestBuffer,
        sizeof(m_requestBuffer),
        "POST %s HTTP/1.1\r\n"
        "HOST: %s\r\n"
        "SOAPACTION: \"urn:schemas-upnp-org:service:%s#%s\"\r\n"
        "CONTENT-TYPE: text/xml; charset=\"utf-8\"\r\n"
        "CONTENT-LENGTH: %d\r\n"
        "\r\n"
        "%s",
        m_controlURL,
        addressBuff,
        m_isIp ? "WANIPConnection:1" : "WANPPPConnection:1",
        command,
        actualSoapSize,
        soapBuffer,
        soapBuffer
    );
}

bdBool bdUPnPDevice::extractURLBase(bdByte8** baseLoc, bdUInt& baseLen)
{
    bdNChar8* slashLoc;
    bdByte8* elementEnd;
    bdNChar8* addressEnd;
    bdNChar8* addressStart;
    bdUInt elementLen;
    bdByte8* elementStart;
    bdByte8 addrString[22];
    bdByte8 urlBase[9];

    *baseLoc = 0LL;
    baseLen = 0;
    elementStart = 0LL;
    elementLen = 0;
    strcpy(urlBase, "<URLBase");
    
    if (!extractTag(urlBase, m_readBuffer, &elementStart, elementLen))
    {
        return true;
    }
    addressStart = bdStrstr(elementStart, "http://");
    if (addressStart != elementStart)
    {
        bdLogWarn("bdnet/upnpdevice", "Description [URLBase tag] is improperly formatted. Missing http://");
        return false;
    }
    elementEnd = &elementStart[elementLen];
    addressStart += bdStrlen("http://");
    slashLoc = bdStrstr(addressStart, "/");

    if (slashLoc >= elementEnd)
    {
        addressEnd = elementEnd;
    }
    else
    {
        *baseLoc = slashLoc;
        baseLen = elementEnd - slashLoc;
        addressEnd = slashLoc;
    }

    if ((addressEnd - addressStart) < sizeof(addrString))
    {
        bdStrlcpy(addrString, addressStart, (addressEnd - addressStart + 1));
        m_deviceAddr.set(addrString);
        return true;
    }
    return false;
}

bdBool bdUPnPDevice::extractServiceType(bdBool& serviceType)
{
    if (bdStrstr(m_readBuffer, "WANIPConnection:1"))
    {
        serviceType = true;
    }
    else if (bdStrstr(m_readBuffer, "WANPPPConnection:1"))
    {
        serviceType = false;
    }
    else
    {
        bdLogWarn("bdnet/upnpdevice", "No service type found.");
        return false;
    }
    return true;
}

bdBool bdUPnPDevice::extractControlURL(bdByte8** controlLoc, bdUInt& controlLen)
{
    bdByte8* elementEnd;
    bdNChar8* addressEnd;
    bdNChar8* addressStart;
    bdNChar8* buffStart;
    bdUInt elementLen;
    bdByte8* elementStart;
    bdByte8 addrString[22];
    bdByte8 control_url[12];

    *controlLoc = 0LL;
    controlLen = 0;
    elementStart = 0LL;
    elementLen = 0;

    if (m_isIp)
    {
        buffStart = bdStrstr(m_readBuffer, "WANIPConnection:1");
    }
    else
    {
        buffStart = bdStrstr(this->m_readBuffer, "WANPPPConnection:1");
    }

    if (!buffStart)
    {
        return false;
    }
    strcpy(control_url, "<controlURL");

    if (!extractTag(control_url, buffStart, &elementStart, elementLen))
    {
        bdLogWarn("bdnet/upnpdevice", "ControlURL specifier tag is missing");
        return false;
    }
    addressStart = bdStrstr(elementStart, "http://");
    elementEnd = &elementStart[elementLen];
    if (addressStart == elementStart)
    {
        addressStart += bdStrlen("http://");
        addressEnd = bdStrstr(addressStart, "/");
        if (!addressEnd || addressEnd > elementEnd)
        {
            addressEnd = elementEnd;
        }
        if ((addressEnd - addressStart) < sizeof(addrString))
        {
            bdStrlcpy(addrString, addressStart, (addressEnd - addressStart + 1));
            m_deviceAddr.set(addrString);
        }
        *controlLoc = addressEnd;
        controlLen = elementEnd - addressEnd;
    }
    else
    {
        *controlLoc = elementStart;
        controlLen = elementLen;
    }
    return true;
}

bdBool bdUPnPDevice::extractExternalAddress(bdInetAddr& externalAddress)
{
    bdByte8 external_ip_address_string[22];
    bdByte8 addrString[22];

    bdMemcpy(&externalAddress, &bdInetAddr(), sizeof(bdInetAddr));
    bdByte8* addrLoc = NULL;
    bdUInt addrLen = 0;

    strcpy(external_ip_address_string, "<NewExternalIPAddress");
    if (!extractTag(external_ip_address_string, m_readBuffer, &addrLoc, addrLen))
    {
        return true;
    }
    if (addrLen && addrLen < sizeof(addrString))
    {
        bdStrlcpy(addrString, addrLoc, addrLen + 1);
        externalAddress.set(addrString);
        bdLogInfo("bdnet/upnpdevice", "External address on device determined to be: %s", addrString);
        return true;
    }
    return false;
}

void bdUPnPDevice::prepareGetMappingsRequest()
{
    bdByte8 queryMapArguments[110];

    bdSnprintf(
        queryMapArguments,
        sizeof(queryMapArguments),
        "<NewRemoteHost></NewRemoteHost>\r\n<NewExternalPort>%d</NewExternalPort>\r\n<NewProtocol>UDP</NewProtocol>\r\n",
        m_gamePort);
    genUPnPCommand("GetSpecificPortMappingEntry", queryMapArguments);
    bdLogInfo("bdnet/upnpdevice", "Querying UPnP device for existing mappings on port %u.", m_gamePort);
}

bdBool bdUPnPDevice::parseGetMappingsResponse(bdBool* mappingExists, bdBool* mappingIsMine)
{
    bdNChar8 addressBuffer[22];
    bdByte8 internal_client_string[19];

    strcpy(internal_client_string, "<NewInternalClient");
    bdByte8* addrLoc = NULL;
    bdUInt addrLen = 0;
    bdBool ok = true;
    *mappingExists = false;
    *mappingIsMine = false;
    *mappingExists = confirmHttpSuccess();

    if (*mappingExists)
    {
        if (!extractTag(internal_client_string, m_readBuffer, &addrLoc, addrLen))
        {
            *mappingExists = 0;
        }
    }
    if (*mappingExists)
    {
        bdInetAddr mappingOwner;
        ok = extractMappingOwner(&mappingOwner, addrLoc, addrLen);
        for (bdUInt i = 0; i < m_localAddrs->getSize() && !*mappingIsMine; ++i)
        {
            bdInetAddr currAddress;
            m_localAddrs->get(i, currAddress);
            if (currAddress == mappingOwner)
            {
                *mappingIsMine = true;
            }
        }
        if (!*mappingIsMine)
        {
            mappingOwner.toString(addressBuffer, sizeof(addressBuffer));
            bdLogInfo("bdnet/upnpdevice", "Port Mapping collision found, owned by %s.", addressBuffer);
        }
    }
    return ok;
}

bdBool bdUPnPDevice::extractMappingOwner(bdInetAddr* owner, bdByte8* addrLoc, bdUInt addrLen)
{
    bdByte8 addrString[22];

    *owner = bdInetAddr();
    if (!addrLen)
    {
        return false;
    }
    if (addrLen < sizeof(addrString))
    {
        bdStrlcpy(addrString, addrLoc, addrLen + 1);
        bdInetAddr mappingOwner(addrString);
        if (mappingOwner.isValid())
        {
            owner->set(mappingOwner);
            return true;
        }
    }
    return false;
}

const bdUPnPDevice::bdUPnPDeviceState bdUPnPDevice::getDeviceState() const
{
    return m_state;
}

const bdUPnPDevice::bdUPnPPortStatus bdUPnPDevice::getPortStatus() const
{
    return m_portStatus;
}

bdAddr bdUPnPDevice::getExternalAddr()
{
    return bdAddr(m_externalDeviceAddr, m_gamePort);
}

bdAddr bdUPnPDevice::getDeviceAddr()
{
    return bdAddr(m_deviceAddr);
}
