// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#define BD_UPNP_MAX_DEVICE_REQUEST_SIZE 0x800

class bdUPnPDevice
{
public:
    enum bdUPnPDeviceState : bdInt
    {
        BD_UPNP_DEVICE_UNINITIALISED = 0x0,
        BD_UPNP_DEVICE_INITIALISED = 0x1,
        BD_UPNP_DEVICE_SENDING_DESCRIPTION_REQ = 0x2,
        BD_UPNP_DEVICE_GETTING_DESCRIPTION = 0x3,
        BD_UPNP_DEVICE_GOT_DESCRIPTION = 0x4,
        BD_UPNP_DEVICE_SENDING_IP_REQ = 0x5,
        BD_UPNP_DEVICE_DISCOVERING_IP = 0x6,
        BD_UPNP_DEVICE_DISCOVERED_IP = 0x7,
        BD_UPNP_DEVICE_SENDING_PORT_QUERY_REQ = 0x8,
        BD_UPNP_DEVICE_DISCOVERING_PORT_MAPPING = 0x9,
        BD_UPNP_DEVICE_DISCOVERED_PORT_MAPPING = 0xA,
        BD_UPNP_DEVICE_SENDING_MAPPING_REQ = 0xB,
        BD_UPNP_DEVICE_MAPPING_PORT = 0xC,
        BD_UPNP_DEVICE_MAPPED_PORT = 0xD,
        BD_UPNP_DEVICE_SENDING_UNMAPPING_REQ = 0xE,
        BD_UPNP_DEVICE_UNMAPPING_PORT = 0xF,
        BD_UPNP_DEVICE_UNMAPPED_PORT = 0x10,
        BD_UPNP_DEVICE_SHUTTING_DOWN = 0x11,
        BD_UPNP_DEVICE_FINISHED = 0x12,
        BD_UPNP_DEVICE_FAILED = 0x13,
    };
    enum bdUPnPDeviceShutdownType : bdInt
    {
        BD_UPNP_DEVICE_SHUTDOWN_ELEGANT = 0x0,
        BD_UPNP_DEVICE_SHUTDOWN_IMMEDIATE = 0x1,
    };
    enum bdUPnPPortStatus : bdInt
    {
        BD_UPNP_PORT_STATUS_UNKNOWN = 0x0,
        BD_UPNP_PORT_COLLISION_FOUND = 0x1,
        BD_UPNP_PORT_NOT_MAPPED = 0x2,
        BD_UPNP_PORT_MAPPED = 0x3,
    };
protected:
    bdUPnPDeviceState m_state;
    bdBool m_shutdownRequested;
    bdUPnPDeviceShutdownType m_shutdownType;
    bdUPnPPortStatus m_portStatus;
    bdAddr m_deviceAddr;
    bdArray<bdInetAddr>* m_localAddrs;
    bdInetAddr m_localConnectedAddr;
    bdStreamSocket m_streamSocket;
    bdBool m_isIp;
    bdUPnPConfig m_config;
    bdUInt16 m_gamePort;
    bdInetAddr m_externalDeviceAddr;
    bdNChar8 m_requestBuffer[2048];
    bdUInt m_requestSize;
    bdNChar8 m_readBuffer[6144];
    bdUInt m_bytesReceived;
    bdNChar8 m_fetchLocation[64];
    bdNChar8 m_controlURL[128];
    bdStopwatch m_connectTimer;
    bdStopwatch m_responseTimer;
public:
    bdUPnPDevice();
    ~bdUPnPDevice();
    bdBool init(bdArray<bdInetAddr>* localAddrs, const bdUByte8* const fetchLocation, const bdInt fetchLen, bdInetAddr* deviceAddr, const bdUInt16 port, const bdUPnPConfig config);
    void pump();
    void cleanup();
    void startShutdown(bdUPnPDeviceShutdownType shutdownType);
    void prepareGetDescriptionRequest();
    bdBool parseDescriptionResponse();
    bdBool connectToDevice();
    bdBool pumpSendRequest(bdBool* isComplete);
    bdBool setLocalConnectedAddr();
    bdBool pumpReceive();
    void prepareGetExternalIPRequest();
    bdBool parseGetExternalIPResponse();
    bdBool confirmHttpSuccess();
    bdBool extractUPnPError(bdInt32* errorCode);
    void selectNewExternalPort();
    void preparePortUnMappingRequest();
    void setupReceive();
    void prepareCreatePortMappingRequest();
    bdBool extractTag(const bdByte8* tag, bdByte8* buffer, bdByte8** value, bdUInt* valueLen);
    bdBool checkForFinishedSoapResponse();
    void genUPnPCommand(const bdByte8* const command, const bdByte8* const arguments);
    bdBool extractURLBase(bdByte8** baseLoc, bdUInt* baseLen);
    bdBool extractServiceType(bdBool* serviceType);
    bdBool extractControlURL(bdByte8** controlLoc, bdUInt* controlLen);
    bdBool extractExternalAddress(bdInetAddr* externalAddress);
    void prepareGetMappingRequest();
    bdBool parseGetMappingsResponse(bdBool* mappingExists, bdBool* mappingIsMine);
    bdBool extractMappingOwner(bdInetAddr* owner, bdByte8* addrLoc, bdUInt addrLen);

    const bdAddr* getDeviceAddr() const;
    const bdAddr* getExternalAddr() const;
    const bdUPnPPortStatus getPortStatus() const;
    const bdUPnPDeviceState getDeviceState() const;
};
