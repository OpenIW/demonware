// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "bdPlatform/bdPlatform.h"

// Thread & Memory headers
#include "bdThread/bdMutex.h"
#include "bdThread/bdRunnable.h"
#include "bdThread/bdSemaphore.h"
#include "bdMemory/bdMemory.h"
#include "bdMemory/bdMemory-std.h"
#include "bdMemory/bdAlignedOffsetMemory.h"
#include "bdMemory/bdMallocMemory.h"
#include "bdMemory/bdMallocMemory-std.h"
#include "bdThread/bdThread.h"

// Reference headers
#include "bdReference/bdReferencable.h"
#include "bdReference/bdReference.h"

// Utilities & Container headers
#include "bdUtilities/bdBitOperations.h"
#include "bdUtilities/bdComparisonFunctors.h"
#include "bdUtilities/bdBase64.h"
#include "bdUtilities/bdBytePacker.h"
#include "bdUtilities/bdRandom.h"
#include "bdContainers/bdFastArray.h"
#include "bdContainers/bdArray.h"
#include "bdContainers/bdSingleton.h"
#include "bdUtilities/bdTrulyRandom.h"
#include "bdUtilities/bdFactory.h"
#include "bdTiming/bdStopwatch.h"

// The rest of the container headers
#include "bdContainers/bdBitBufferTypes.h"
#include "bdContainers/bdBitBuffer.h"
#include "bdContainers/bdByteBuffer.h"
#include "bdContainers/bdCircularBuffer.h"
#include "bdContainers/bdHashMap.h"
#include "bdContainers/bdLinkedList.h"
#include "bdContainers/bdNonce.h"
#include "bdContainers/bdPriorityHeap.h"
#include "bdContainers/bdPriorityQueue.h"
#include "bdContainers/bdQueue.h"
#include "bdContainers/bdRedBlacktree.h"
#include "bdContainers/bdSequenceNumber.h"
#include "bdContainers/bdSequenceNumberStore.h"
#include "bdContainers/bdSet.h"

// String headers
#include "bdString/bdString.h"

// Cryptography headers
#include <tomcrypt.h>
#include "bdCrypto/bdCryptoConfig.h"
#include "bdCrypto/bdCrypto.h"

// Socket headers
#include "bdSocket/bdSocketConfig.h"
#include "bdSocket/bdPort.h"
#include "bdSocket/bdSecurityID.h"
#include "bdSocket/bdSecurityKey.h"
#include "bdSocket/bdSecurityInfo.h"
#include "bdSocket/bdInetAddr.h"
#include "bdSocket/bdAddr.h"
#include "bdSocket/bdCommonAddr.h"
#include "bdSocket/bdCommonAddrInfo.h"
#include "bdSocket/bdEndpoint.h"
#include "bdSocket/bdSocket.h"
#include "bdSocket/bdStreamSocket.h"

class bdCore
{
public:
    static bdBool m_initialized;
    static void init(const bdBool defaultMemoryFunctions);
    static void quit();
};
