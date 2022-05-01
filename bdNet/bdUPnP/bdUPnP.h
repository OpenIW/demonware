// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdUPnP
{
public:
    enum bdUPnPState : bdInt
    {
        BD_UPNP_UNINITIALISED = 0x0,
        BD_UPNP_INITIALISED = 0x1,
        BD_UPNP_DISCOVERING_DEVICES = 0x2,
        BD_UPNP_DISCOVERED_DEVICE = 0x3,
        BD_UPNP_CONFIGURING_DEVICE = 0x4,
        BD_UPNP_SHUTTING_DOWN = 0x5,
        BD_UPNP_FINISHED = 0x6,
        BD_UPNP_FAILED = 0x7,
    };
protected:
    bdUPnP::bdUPnPState m_state;
    bdPort m_gamePort;
    bdArray<bdInetAddr>* m_localAddrs;
    bdUPnPConfig m_config;
    bdSocket m_discoverySocket;
    bdNChar8 m_readBuffer[1025];
    bdUInt m_bytesReceived;
    bdUPnPDevice m_device;
    bdUPnPDevice m_cachedDevice;
    bdStopwatch m_discoveryTimer;
    bdUInt m_numDiscoveryAttempts;
    bdBool m_shutdownRequested;
public:
    bdUPnP();
    ~bdUPnP();
    bdBool init(bdPort gamePort, bdArray<bdInetAddr>* localAddrs, bdUPnPConfig* config);
    void pump();
    void cleanup();
    void startShutdown(bdUPnPDevice::bdUPnPDeviceShutdownType shutdownType);
    bdBool startDiscovery();
    bdBool setupDiscoverySocket();
    bdBool handleDiscoveryResponse(bdUPnPDevice* discoveredDevice);
    bdBool checkForDiscoveredDevices(bdUPnPDevice* discoveredDevice);
    bdBool extractDeviceInfo(bdAddr* deviceAddr, bdByte8** fetchLoc, bdUInt* fetchLen);
    bdBool getExternalAddr(bdAddr* externalAddr);

    const bdBool isRunning() const;
    const bdUPnPDevice::bdUPnPPortStatus getPortStatus() const;
    const bdUPnP::bdUPnPState getState() const;
};