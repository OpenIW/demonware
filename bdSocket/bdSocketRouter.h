// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "bdCore/bdCore.h"

#include "bdServiceBandwidthArbitrator.h"
#include "bdNAT/bdNATTypeDiscoveryConfig.h"
#include "bdNAT/bdPacket/bdNATTypeDiscoveryPacket.h"
#include "bdNAT/bdPacket/bdNATTypeDiscoveryPacketReply.h"
#include "bdNAT/bdIPDiscoveryConfig.h"
#include "bdNAT/bdPacket/bdIPDiscoveryPacket.h"
#include "bdNAT/bdPacket/bdIPDiscoveryPacketReply.h"
#include "bdNAT/bdNAT.h"
#include "bdNAT/bdNATTypeDiscoveryClient.h"
#include "bdNAT/bdIPDiscoveryClient.h"
#include "bdNAT/bdPacket/bdNATTraversalPacket.h"
#include "bdNAT/bdNATTravListener.h"
#include "bdNAT/bdNATTravClientData.h"
#include "bdPacketInterceptor.h"
#include "bdNAT/bdNATTravClient.h"

#include "bdQoS/bdQoSProbeListener.h"
#include "bdQoS/bdQoSRemoteAddr.h"
#include "bdQoS/bdPacket/bdQoSReplyPacket.h"
#include "bdQoS/bdQoSProbe.h"

// class bdSocketRouter;

#include "bdSocketRouterConfig.h"