// SPDX-License-Identifier: GPL-3.0-or-later

class bdNetStartParams
{
protected:
    bdBool m_onlineGame;
    bdUInt16 m_gamePort;
    bdSocket* m_socket;
    bdArray<bdString> m_natTravHosts;
    bdUInt16 m_natTravPort;
    bdArray<bdInetAddr> m_localAddresses;
    //bdGetHostByNameConfig m_hostNameLookupConfig;
    //bdUPnPConfig m_UPnPConfig;
    bdBool m_useAnyIP;
};