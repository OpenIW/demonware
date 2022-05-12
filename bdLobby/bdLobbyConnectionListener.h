// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdLobbyConnection;
typedef bdReference<bdLobbyConnection> bdLobbyConnectionRef;

class bdLobbyConnectionListener : bdReferencable
{
public:
    virtual ~bdLobbyConnectionListener();
    virtual void onConnect(bdLobbyConnectionRef);
    virtual void onConnectFailed(bdLobbyConnectionRef);
    virtual void onDisconnect(bdLobbyConnectionRef);
};
