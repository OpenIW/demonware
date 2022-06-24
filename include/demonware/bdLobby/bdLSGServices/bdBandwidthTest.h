// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

enum bdBandwidthTestStatus
{
    BD_BANDWIDTH_TEST_IDLE = 0,
    BD_BANDWIDTH_TEST_REQUESTING_TEST = 1,
    BD_BANDWIDTH_TEST_UPLOAD_WAITING = 2,
    BD_BANDWIDTH_TEST_UPLOAD_SENDING = 3,
    BD_BANDWIDTH_TEST_DOWNLOAD_WAITING = 4,
    BD_BANDWIDTH_TEST_DOWNLOAD_RECEIVING = 5,
    BD_BANDWIDTH_TEST_FINALIZING = 6,
    BD_BANDWIDTH_TEST_DONE = 7
};

enum bdBandwidthTestType
{
    BD_UPLOAD_TEST = 0,
    BD_UPLOAD_DOWNLOAD_TEST = 1
};

enum bdBandwidthTestInitStatus
{
    BD_BANDWIDTH_TEST_UNINITIALIZED = 0,
    BD_BANDWIDTH_TEST_INITIALIZED = 1
};

class bdBandwidthTestClient
{
protected:
    bdSocket m_socket;
    bdRemoteTaskRef m_remoteTask;
    bdUInt m_packetSize;
    bdUInt m_numPackets;
    bdUInt m_senderInitialWait;
    bdUInt m_sendDuration;
    bdUInt m_receiverInitialWait;
    bdUInt m_receiveDuration;
    bdUInt m_lingerDuration;
    bdUInt16 m_lsgPort;
    bdUInt m_lsgAddr;
    bdUByte8 m_cookie[8];
    bdStopwatch m_timer;
    bdStopwatch m_recvTimeoutTimer;
    bdUByte8* m_packetBuffer;
    bdUInt m_nextPacketNum;
    bdUInt m_actualNumSent;
    bdUInt m_seqNumberTotal;
    bdLobbyService* m_lobbyService;
    bdBandwidthTestInitStatus m_initStatus;
    bdBandwidthTestStatus m_testStatus;
    bdBandwidthTestResults m_uploadResults;
    bdBandwidthTestResults m_downloadResults;
    bdLobbyErrorCode m_error;
    bdBandwidthTestType m_type;
public:
    bdBandwidthTestClient();
    ~bdBandwidthTestClient();
    bdBool init(bdLobbyService* const lobbyService);
    void quit();
    void start(const bdBandwidthTestType type);
    void stop();
    bdBandwidthTestStatus getStatus();
    void pump();
    bdLobbyErrorCode getErrorCode();
    bdBandwidthTestType getTestType();
    bdBandwidthTestResults& getUploadResults();
    bdBandwidthTestResults& getDownloadResults();
protected:
    void pumpRequest();
    void handleRequestReply(const bdByteBufferRef replyBuffer);
    void startUploadTest();
    void pumpUploadWait();
    void pumpUploadSend();
    void startDownloadTest();
    void pumpDownloadReceive();
    void finalizeTest();
    void pumpFinalize();
    void handleFinalizeReply(const bdByteBufferRef replyBuffer);
};
