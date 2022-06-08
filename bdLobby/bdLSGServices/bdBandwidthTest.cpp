// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

bdBandwidthTestClient::bdBandwidthTestClient()
    : m_socket(), m_remoteTask(), m_timer(), m_recvTimeoutTimer(), m_packetBuffer(BD_NULL), m_nextPacketNum(0), m_actualNumSent(0), m_initStatus(BD_BANDWIDTH_TEST_UNINITIALIZED),
    m_testStatus(BD_BANDWIDTH_TEST_IDLE), m_uploadResults(), m_downloadResults(), m_error(BD_NO_ERROR), m_type(BD_UPLOAD_TEST)
{
}

bdBandwidthTestClient::~bdBandwidthTestClient()
{
    stop();
    quit();
}

bdBool bdBandwidthTestClient::init(bdLobbyService* const lobbyService)
{
    bdBool ok = false;
    m_error = BD_BANDWIDTH_TEST_SOCKET_ERROR;

    if (m_initStatus)
    {
        bdLogWarn("bdBandwidthTestClient", "bdBandwidthTestClient already initialized.");
        return ok;
    }
    ok = m_socket.create(false, false);
    if (!ok)
    {
        bdLogError("bdBandwidthTestClient", "bdBandwidthTestClient failed to create UDP socket.");
        return ok;
    }
    m_error = BD_NO_ERROR;
    m_initStatus = BD_BANDWIDTH_TEST_INITIALIZED;
    m_lobbyService = lobbyService;
    bdLogInfo("bdBandwidthTestClient", "Initialised client.");
    return ok;
}

void bdBandwidthTestClient::quit()
{
    if (!m_initStatus)
    {
        bdLogInfo("bdBandwidthTestClient", "quit called on uninitialized bdBandwidthTestClient instance.");
        return;
    }

    if (!m_socket.close())
    {
        bdLogError("bdBandwidthTestClient", "bdBandwidthTestClient failed to close UDP socket.");
    }
    m_initStatus = BD_BANDWIDTH_TEST_UNINITIALIZED;
    bdLogInfo("bdBandwidthTestClient", "Quit client.");
}

void bdBandwidthTestClient::start(const bdBandwidthTestType type)
{
    m_error = BD_START_TASK_FAILED;

    if (!m_initStatus)
    {
        bdLogWarn("bdBandwidthTestClient", "bdBandwidthTestClient not initialized.");
        return;
    }
    if (m_testStatus != BD_BANDWIDTH_TEST_IDLE && m_testStatus != BD_BANDWIDTH_TEST_DONE)
    {
        bdLogWarn("bdBandwidthTestClient", "bdBandwidthTestClient already in use.");
        return;
    }
    if (!m_lobbyService)
    {
        bdLogWarn("bdBandwidthTestClient", "Cannot start test as lobby service pointer is invalid.");
        return;
    }

    bdRemoteTaskManager* taskManager = m_lobbyService->getTaskManager();
    if (!taskManager)
    {
        bdLogWarn("bdBandwidthTestClient", "Failed to retreive task manager from lobby service.");
        return;
    }

    bdUByte8 buffer[16];
    bdUInt offset = 0;
    bdUByte8 requestType8 = 0;
    bdUByte8 testType8 = type;
    bdBool appended = bdBytePacker::appendBasicType<bdUByte8>(buffer, sizeof(buffer), 0, offset, requestType8);
    appended = appended == bdBytePacker::appendBasicType<bdUByte8>(buffer, sizeof(buffer), offset, offset, testType8);

    if (!appended)
    {
        bdLogWarn("bdBandwidthTestClient", "Failed to pack request buffer.");
        return;
    }

    m_error = taskManager->startLSGTask(m_remoteTask, 18, 1, buffer, sizeof(buffer));
    if (m_error)
    {
        bdLogWarn("bdBandwidthTestClient", "Failed to start task for bandwidth test of type: %u", m_type);
        return;
    }

    m_testStatus = BD_BANDWIDTH_TEST_REQUESTING_TEST;
    m_type = type;
    m_uploadResults.clear();
    m_downloadResults.clear();
    bdLogInfo("bdBandwidthTestClient", "Started bandwidth test of type: %u.", m_type);
}

void bdBandwidthTestClient::stop()
{
    bdLogInfo("bdBandwidthTestClient", "Stopping bandwidth test of type: %u.", m_type);
    if (m_remoteTask.notNull())
    {
        m_remoteTask = BD_NULL;
    }
    if (m_packetBuffer)
    {
        bdDeallocate<bdUByte8>(m_packetBuffer);
        m_packetBuffer = BD_NULL;
    }
    m_testStatus = BD_BANDWIDTH_TEST_IDLE;
    m_error = BD_NO_ERROR;
}

bdBandwidthTestStatus bdBandwidthTestClient::getStatus()
{
    return m_testStatus;
}

void bdBandwidthTestClient::pump()
{
    if (m_initStatus != BD_BANDWIDTH_TEST_UNINITIALIZED)
    {
        bdLogWarn("bdBandwidthTestClient", "Pumping uninitialized bdBandwidthTestClient");
    }
    switch (m_testStatus)
    {
    case BD_BANDWIDTH_TEST_IDLE:
        bdLogWarn("bdBandwidthTestClient", "Pumping idle bdBandwidthTestClient.");
        break;
    case BD_BANDWIDTH_TEST_REQUESTING_TEST:
        pumpRequest();
        break;
    case BD_BANDWIDTH_TEST_UPLOAD_WAITING:
        pumpUploadWait();
        break;
    case BD_BANDWIDTH_TEST_UPLOAD_SENDING:
        pumpUploadSend();
        break;
    case BD_BANDWIDTH_TEST_DOWNLOAD_WAITING:
    case BD_BANDWIDTH_TEST_DOWNLOAD_RECEIVING:
        pumpDownloadReceive();
        break;
    case BD_BANDWIDTH_TEST_FINALIZING:
        pumpFinalize();
        break;
    default:
        return;
    }
}

bdLobbyErrorCode bdBandwidthTestClient::getErrorCode()
{
    return m_error;
}

bdBandwidthTestType bdBandwidthTestClient::getTestType()
{
    return m_type;
}

bdBandwidthTestResults& bdBandwidthTestClient::getUploadResults()
{
    return m_uploadResults;
}

bdBandwidthTestResults& bdBandwidthTestClient::getDownloadResults()
{
    return m_downloadResults;
}

void bdBandwidthTestClient::pumpRequest()
{
    bdAssert(*m_remoteTask != BD_NULL, "bdBandwidthTestClient status is BD_REQUESTING_TEST with null task handle.");
    bdAssert(m_testStatus == BD_BANDWIDTH_TEST_REQUESTING_TEST, "bdBandwidthTestClient::pumpRequest called in incorrect status.");
    m_error = BD_HANDLE_TASK_FAILED;

    if (m_remoteTask.notNull() && m_testStatus == BD_BANDWIDTH_TEST_REQUESTING_TEST)
    {
        switch (m_remoteTask->getStatus())
        {
            case bdRemoteTask::BD_EMPTY:
                bdLogError("bdBandwidthTestClient", "bdBandwidthTestClient status is BD_BANDWIDTH_TEST_REQUESTING_TEST but task is bdRemoteTask::BD_EMPTY.");
                break;
            case bdRemoteTask::BD_PENDING:
                m_error = BD_NO_ERROR;
                break;
            case bdRemoteTask::BD_DONE:
            {
                m_error = BD_NO_ERROR;
                bdByteBufferRef replyBuffer = m_remoteTask->getByteResults();
                m_remoteTask = BD_NULL;
                handleRequestReply(bdByteBufferRef(replyBuffer));
                break;
            }
            case bdRemoteTask::BD_TIMED_OUT:
                bdLogWarn("bdBandwidthTestClient", "bdBandwidthTestClient test request timed out.");
                break;
            case bdRemoteTask::BD_MAX_STATUS:
                bdLogError("bdBandwidthTestClient", "bdBandwidthTestClient test request invalid remote task state (bdRemoteTask::BD_MAX_STATUS).");
                break;
            default:
                break;
        }
    }
}

void bdBandwidthTestClient::handleRequestReply(const bdByteBufferRef replyBuffer)
{
    m_error = BD_HANDLE_TASK_FAILED;
    if (replyBuffer.notNull())
    {
        bdUByte8 testRejected = 1;
        bdBool ok = replyBuffer->read<bdUByte8>(testRejected);
        if (ok && testRejected == 1)
        {
            bdUInt16 errorCode = 0;
            if (replyBuffer->read<bdUInt16>(errorCode))
            {
                m_error = static_cast<bdLobbyErrorCode>(errorCode);
            }
            else
            {
                bdLogError("bdBandwidthTestClient", "Failed to parse bandwidth test error code.");
            }
            bdLogInfo("bdBandwidthTestClient", "Request for bandwidth test rejected (%u).", m_error);
        }
        if (ok && testRejected == 0)
        {
            bdUInt32 lsgAddr = 0;
            ok = replyBuffer->read<bdUInt>(m_packetSize);
            ok = ok == replyBuffer->read<bdUInt>(m_numPackets);
            ok = ok == replyBuffer->read<bdUInt>(m_senderInitialWait);
            ok = ok == replyBuffer->read<bdUInt>(m_sendDuration);
            ok = ok == replyBuffer->read<bdUInt>(m_receiverInitialWait);
            ok = ok == replyBuffer->read<bdUInt>(m_receiveDuration);
            ok = ok == replyBuffer->read<bdUInt>(m_lingerDuration);
            ok = ok == replyBuffer->read<bdUInt16>(m_lsgPort);
            ok = ok == replyBuffer->read<bdUInt>(lsgAddr);

            bdBitOperations::endianSwap<bdUInt>(lsgAddr, m_lsgAddr);
            bdMemset(m_cookie, 0, sizeof(m_cookie));
            for (int i = 0; i < 8 && ok; ++i)
            {
                ok = replyBuffer->read<bdUByte8>(m_cookie[i]);
            }
            m_packetBuffer = bdReallocate<bdUByte8>(m_packetBuffer, m_packetSize);
            if (ok && m_packetBuffer)
            {
                m_error = BD_NO_ERROR;
                startUploadTest();
            }
            else
            {
                bdLogError("bdBandwidthTestClient", "Failed to parse bandwidth test reply.");
            }
        }
    }
}

void bdBandwidthTestClient::startUploadTest()
{
    bdLogInfo(
        "bdBandwidthTestClient", "Starting upload test: m_packetSize:%u m_numPackets:%u m_senderInitialWait:%u m_sendDuration:%u m_receiverInitialWait"
        ":%u m_receiveDuration:%u m_lingerDuration:%u m_lsgPort:%u m_lsgAddr:%u ",
        m_packetSize,
        m_numPackets,
        m_senderInitialWait,
        m_sendDuration,
        m_receiverInitialWait,
        m_receiveDuration,
        m_lingerDuration,
        m_lsgPort,
        m_lsgAddr);
    m_timer.start();
    m_testStatus = BD_BANDWIDTH_TEST_UPLOAD_WAITING;
}

void bdBandwidthTestClient::pumpUploadWait()
{
    if (m_timer.getElapsedTimeInSeconds() >= (m_senderInitialWait / 1000.0))
    {
        m_testStatus = BD_BANDWIDTH_TEST_UPLOAD_SENDING;
        m_nextPacketNum = 0;
        m_timer.start();
    }
}

void bdBandwidthTestClient::pumpUploadSend()
{
    bdFloat32 sendDurationSecs = m_sendDuration / 1000.0f;
    bdFloat32 sendTimeProportion = m_timer.getElapsedTimeInSeconds() / sendDurationSecs;

    sendTimeProportion = sendTimeProportion > 1.0 ? 1.0 : (sendTimeProportion < 0.0 ? 0.0 : sendTimeProportion);
    bdUInt firstPacket = m_nextPacketNum;
    bdFloat32 lastPacket = m_numPackets * sendTimeProportion;
    bdBool sent = true;
    while (sent && lastPacket > m_nextPacketNum)
    {
        sent = false;
        bdUInt offset = 0;
        bdBool appended = bdBytePacker::appendBasicType<bdUInt>(m_packetBuffer, m_packetSize, 0, offset, m_nextPacketNum);
        appended = appended == bdBytePacker::appendBuffer(m_packetBuffer, m_packetSize, offset, offset, m_cookie, sizeof(m_cookie));
        if (appended && offset < m_packetSize)
        {
            bdRandom rand;
            rand.nextUBytes(&m_packetBuffer[offset], m_packetSize - offset);
        }

        if (appended)
        {
            bdAddr destAddr(bdInetAddr(m_lsgAddr), m_lsgPort);
            bdInt bytesSent = m_socket.sendTo(destAddr, m_packetBuffer, m_packetSize);
            if (bytesSent > 0 && m_packetSize == bytesSent)
            {
                sent = true;
                ++m_nextPacketNum;
                ++m_actualNumSent;
            }
        }
        else
        {
            bdLogError("bdBandwidthTestClient", "Failed to create UDP packet.");
        }
    }
    sendTimeProportion = m_timer.getElapsedTimeInSeconds() / sendDurationSecs;
    bdUInt lastPacketByNow = (sendTimeProportion > 1.0 ? 1.0 : (sendTimeProportion < 0.0 ? 0.0 : sendTimeProportion)) * m_numPackets;
    if (m_nextPacketNum != firstPacket && m_nextPacketNum < lastPacketByNow && m_nextPacketNum - firstPacket < lastPacketByNow - m_nextPacketNum)
    {
        m_nextPacketNum = lastPacketByNow;
    }
    if (m_nextPacketNum == m_numPackets)
    {
        bdLogInfo("bdBandwidthTestClient", "Upload bandwidth test took %.2fs to send %u/%u packets of %u bytes each.",
            m_timer.getElapsedTimeInSeconds(), m_actualNumSent, m_numPackets, m_packetSize);
        if (m_type == BD_UPLOAD_TEST)
        {
            finalizeTest();
        }
        if (m_type == BD_UPLOAD_DOWNLOAD_TEST)
        {
            startDownloadTest();
        }
    }
}

void bdBandwidthTestClient::startDownloadTest()
{
    bdLogInfo(
        "bdBandwidthTestClient",
        "Starting download test: m_packetSize:%u m_numPackets:%u m_senderInitialWait:%u m_sendDuration:%u m_receiverInitialWa"
        "it:%u m_receiveDuration:%u m_lingerDuration:%u m_lsgPort:%u m_lsgAddr:%u ",
        m_packetSize,
        m_numPackets,
        m_senderInitialWait,
        m_sendDuration,
        m_receiverInitialWait,
        m_receiveDuration,
        m_lingerDuration,
        m_lsgPort,
        m_lsgAddr);
    m_testStatus = BD_BANDWIDTH_TEST_DOWNLOAD_WAITING;
    m_timer.start();
    m_seqNumberTotal = 0;
}

void bdBandwidthTestClient::pumpDownloadReceive()
{
    bdInt received = 1;
    while (received > 0)
    {
        bdAddr fromAddr;
        received = m_socket.receiveFrom(fromAddr, m_packetBuffer, m_packetSize);
        if (received <= 0)
        {
            continue;
        }
        bdAddr lsgAddr(bdInetAddr(m_lsgAddr), m_lsgPort);
        bdBool ok = received == m_packetSize && fromAddr == lsgAddr;

        bdUInt sequenceNumber = m_numPackets;
        bdUInt offset = 0;
        ok = ok == bdBytePacker::removeBasicType<bdUInt>(m_packetBuffer, received, offset, offset, sequenceNumber);
        ok = ok == sequenceNumber < m_numPackets;
        for (bdUInt cookieBytesRead = 0; cookieBytesRead < sizeof(m_cookie) && ok; ++cookieBytesRead)
        {
            bdUByte8 cookieByte = 0;
            ok = ok == bdBytePacker::removeBasicType<bdUByte8>(m_packetBuffer, received, offset, offset, cookieByte);
            ok = ok == (cookieByte == m_cookie[cookieBytesRead]);
        }
        if (!ok)
        {
            bdLogWarn("bdBandwidthTestClient", "Received malformed bandwidth test packet.");
            continue;
        }
        if (!m_downloadResults.m_bytesReceived)
        {
            m_testStatus = BD_BANDWIDTH_TEST_DOWNLOAD_RECEIVING;
            m_timer.start();
            bdAssert(m_downloadResults.m_bytesReceived == 0, "Bandwidth test download results not cleared.");
            bdAssert(m_downloadResults.m_receivePeriodMs == 1, "Bandwidth test download results not cleared.");
            m_downloadResults.m_avgSeqNumber = sequenceNumber;
            m_downloadResults.m_minSeqNumber = sequenceNumber;
            m_downloadResults.m_maxSeqNumber = sequenceNumber;
        }
        m_downloadResults.m_bytesReceived += received + 8;
        m_downloadResults.m_receivePeriodMs = m_timer.getElapsedTimeInSeconds() * 1000.0;
        m_downloadResults.m_minSeqNumber = m_downloadResults.m_minSeqNumber < sequenceNumber ? m_downloadResults.m_minSeqNumber : sequenceNumber;
        m_downloadResults.m_maxSeqNumber = m_downloadResults.m_maxSeqNumber > sequenceNumber ? m_downloadResults.m_maxSeqNumber : sequenceNumber;
        bdUInt32 numPacketsReceived = m_downloadResults.m_bytesReceived / (m_packetSize + 8);
        m_seqNumberTotal += sequenceNumber;
        m_downloadResults.m_avgSeqNumber = m_seqNumberTotal / numPacketsReceived;
        m_recvTimeoutTimer.start();
    }

    if (m_testStatus == BD_BANDWIDTH_TEST_DOWNLOAD_WAITING)
    {
        bdUInt serverReceiveTimeout = 0;
        if (m_receiveDuration > m_sendDuration)
        {
            serverReceiveTimeout = m_receiveDuration - m_sendDuration;
        }
        bdUInt firstPacketTimeout = serverReceiveTimeout + m_receiverInitialWait;
        if (m_timer.getElapsedTimeInSeconds() >= (firstPacketTimeout / 1000.0))
        {
            bdLogWarn("bdBandwidthTestClient", "Download test timed out. Failed to receive any download packets in %.2fs.",
                firstPacketTimeout / 1000.0);
            finalizeTest();
        }
    }
    if (m_testStatus == BD_BANDWIDTH_TEST_DOWNLOAD_RECEIVING
        && m_receiveDuration / 1000.0 <= m_timer.getElapsedTimeInSeconds()
        && (m_lingerDuration + m_downloadResults.m_receivePeriodMs) / 1000.0 <= m_timer.getElapsedTimeInSeconds())
    {
        bdLogInfo("bdBandwidthTestClient", "Download test complete. Received %u bytes in %.2fs.",
            m_downloadResults.m_bytesReceived, m_downloadResults.m_receivePeriodMs / 1000.0);
        finalizeTest();
    }
}

void bdBandwidthTestClient::finalizeTest()
{
    m_error = BD_START_TASK_FAILED;
    m_testStatus = BD_BANDWIDTH_TEST_FINALIZING;

    if (!m_lobbyService)
    {
        bdLogWarn("bdBandwidthTestClient", "Finalize bandwidth test failed, lobby service invalid.");
        return;
    }

    bdRemoteTaskManager* taskManager = m_lobbyService->getTaskManager();
    if (!taskManager)
    {
        bdLogWarn("bdBandwidthTestClient", "Failed to retreive task manager from lobby service.");
        return;
    }

    bdUByte8 buffer[21];
    bdUInt offset = 0;
    bdUByte8 requestType = 1;
    bdBool appended = bdBytePacker::appendBasicType<bdUByte8>(buffer, sizeof(buffer), 0, offset, requestType);
    appended = appended == m_downloadResults.serialize(&buffer[offset], sizeof(buffer) - offset);

    if (!appended)
    {
        bdLogWarn("bdBandwidthTestClient", "Failed to pack request buffer for finalize bandwidth test.");
        return;
    }

    m_error = taskManager->startLSGTask(m_remoteTask, 18, 1, buffer, 21);
    if (m_error)
    {
        bdLogWarn("bdBandwidthTestClient", "Failed to start task for finalize bandwidth test.");
        return;
    }

    bdLogInfo("bdBandwidthTestClient", "Finalize bandwidth test task started.");
}

void bdBandwidthTestClient::pumpFinalize()
{
    bdAssert(*m_remoteTask != BD_NULL, "bdBandwidthTestClient status is BD_REQUESTING_TEST with null task handle.");
    bdAssert(m_testStatus == BD_BANDWIDTH_TEST_FINALIZING, "bdBandwidthTestClient::pumpFinalize called in incorrect status.");
    m_error = BD_HANDLE_TASK_FAILED;
    if (m_remoteTask.notNull() && m_testStatus == BD_BANDWIDTH_TEST_FINALIZING)
    {
        switch (m_remoteTask->getStatus())
        {
            switch (m_remoteTask->getStatus())
            {
            case bdRemoteTask::BD_EMPTY:
                bdLogError("bdBandwidthTestClient", "bdBandwidthTestClient status is BD_BANDWIDTH_TEST_FINALIZING but task is bdRemoteTask::BD_EMPTY.");
                break;
            case bdRemoteTask::BD_PENDING:
                m_error = BD_NO_ERROR;
                break;
            case bdRemoteTask::BD_DONE:
            {
                m_error = BD_NO_ERROR;
                bdByteBufferRef replyBuffer = m_remoteTask->getByteResults();
                m_remoteTask = BD_NULL;
                handleFinalizeReply(bdByteBufferRef(replyBuffer));
                break;
            }
            case bdRemoteTask::BD_TIMED_OUT:
                bdLogWarn("bdBandwidthTestClient", "bdBandwidthTestClient test request timed out.");
                break;
            case bdRemoteTask::BD_MAX_STATUS:
                bdLogError("bdBandwidthTestClient", "bdBandwidthTestClient finalize request invalid remote task state (bdRemoteTask::BD_MAX_STATUS).");
                break;
            default:
                break;
            }
        }
    }
}

void bdBandwidthTestClient::handleFinalizeReply(const bdByteBufferRef replyBuffer)
{
    m_error = BD_HANDLE_TASK_FAILED;
    if (replyBuffer.notNull())
    {
        bdUByte8 finalizeFailed = 1;
        bdBool ok = replyBuffer->read<bdUByte8>(finalizeFailed);
        if (ok && finalizeFailed == 1)
        {
            bdUInt16 errorCode = 0;
            if (replyBuffer->read<bdUInt16>(errorCode))
            {
                m_error = static_cast<bdLobbyErrorCode>(errorCode);
            }
            else
            {
                bdLogError("bdBandwidthTestClient", "Failed to parse bandwidth test error code.");
            }
            bdLogInfo("bdBandwidthTestClient", "Request for bandwidth test rejected (%u).", m_error);
        }
        if (ok && !finalizeFailed)
        {
            ok = m_uploadResults.deserialize(bdByteBufferRef(replyBuffer));
            if (ok)
            {
                bdLogInfo("bdBandwidthTestClient", "Bandwidth test finalize complete.");
                m_error = BD_NO_ERROR;
                m_testStatus = BD_BANDWIDTH_TEST_DONE;
            }
        }
    }
}
