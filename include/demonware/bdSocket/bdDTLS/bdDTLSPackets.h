// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

enum bdDTLSPacketTypes
{
	BD_DTLS_NULL,
	BD_DTLS_INIT,
	BD_DTLS_INIT_ACK,
	BD_DTLS_COOKIE_ECHO,
	BD_DTLS_COOKIE_ACK,
	BD_DTLS_ERROR,
	BD_DTLS_DATA,
};

#include "bdPacket/bdDTLSHeader.h"
#include "bdPacket/bdDTLSInitAck.h"
#include "bdPacket/bdDTLSCookieAck.h"
#include "bdPacket/bdDTLSCookieEcho.h"
#include "bdPacket/bdDTLSData.h"
#include "bdPacket/bdDTLSError.h"
#include "bdPacket/bdDTLSInit.h"
