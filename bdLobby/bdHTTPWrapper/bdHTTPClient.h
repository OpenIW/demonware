// SPDX-License-Identifier: GPL-3.0-or-later

class bdHTTPClient
{
public:
    enum BD_HTTP_OPERATION
    {
        BD_HTTP_GET_OPERATION = 0,
        BD_HTTP_PUT_OPERATION = 1,
        BD_HTTP_POST_OPERATION = 2,
        BD_HTTP_DELETE_OPERATION = 3,
        BD_HTTP_COPY_OPERATION = 4
    };
    enum BD_HTTP_STATUS
    {
        BD_HTTP_STATUS_READY = 0,
        BD_HTTP_STATUS_BUSY = 1,
        BD_HTTP_STATUS_DONE = 2,
        BD_HTTP_STATUS_ERROR = 3
    };
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdHTTPClient();
    ~bdHTTPClient();
protected:
    bdBool performOperation(bdStreamSocket* socket);
public:
    void performOperation();
    bdBool httpGet(bdNChar8* const serverName, bdNChar8* const downloadURL, void* const getBuffer, bdUInt bufferSize, bdDownloadInterceptor* downloadInterceptor, bdUInt port,
        bdUInt64 transactionID, bdUInt startByte, bdUInt endByte);
    bdBool httpPut(bdNChar8* const serverName, bdNChar8* const uploadURL, const void* uploadData, bdInt uploadSize, bdUploadInterceptor* uploadInterceptor, bdUInt port,
        bdUInt64 transactionID, bdNChar8* const checksum);
    bdBool httpDelete(bdNChar8* const serverName, bdNChar8* const deleteURL, bdUInt port, bdUInt64 transactionID);
    bdBool httpCopy(bdNChar8* const serverName, bdNChar8* const copyURL, const bdNChar8* destination, bdUInt port, bdUInt64 transactionID);
    void enableVerboseOutput(bdBool enable);
    void enableProgressMeter(bdBool enable);
    bdUInt64 getResponseContentDataLength();
    void setResponseContentDataLength(bdUInt64);
    bdHTTPClient::BD_HTTP_STATUS getStatus();
    void setStatus(bdHTTPClient::BD_HTTP_STATUS);
    bdFloat32 getDataRate();
    void setDataRate(bdFloat32 dataRate);
    bdUInt64 getResponseCode();
    bdInt64 getSocketErrorCode();
    void setSocketErrorCode(bdInt64);
    bdHTTPBuffer* getInternalBuffer();
    bdHTTPClient::BD_HTTP_OPERATION getOperation();
    bdUInt64 getExpectedContentSize();
    void setBytesTransfered(bdUInt64 bytes);
    bdUInt64 getBytesTransfered();
    void abortOperation();
    bdBool isAborted();
    bdInt getSocketSendBuffSize();
    void setSocketSendBuffSize(bdInt);
    bdBool parseIncomingHttpResponse(bdNChar8* const responseBuffer, bdUInt bufferSize);
    bdBool parseStatusLine(bdNChar8* httpBuffer, bdUInt bufferSize);
    void clearHttpOperation();
    bdBool initHttpOperation(const bdNChar8* serverName, bdUInt port);
    bdBool validateRequestInput(const bdNChar8* serverName, const bdNChar8* requestPath, const void* inputBuffer, bdInt64 bufferSize, bdBool validInterceptor);
    bdBool resolveHostIP(bdAddr*);
    bdBool sendPayload(bdStreamSocket* sock, bdStopwatch* httpTime);
    bdInt writePayloadData(bdStreamSocket* sock, bdUInt* totalSent);
    bdBool recvResponseHeader(bdStreamSocket* sock, bdUInt* bytesDownloaded);
    bdBool recvResponsePayload(bdStreamSocket*, bdUInt);
    bdSocketStatusCode readUntilStr(bdStreamSocket* socket, const bdNChar8* endString, bdNChar8* recvData, bdUInt maxSize, bdUInt* bufSize, bdInt* endSize);
protected:
    bdNChar8 m_serverName[128];
    bdPort m_port;
    bdDownloadInterceptor* m_downloadHandler;
    bdUploadInterceptor* m_uploadHandler;
    bdHTTPClient::BD_HTTP_OPERATION m_operation;
    bdHTTPClient::BD_HTTP_STATUS m_status;
    bdHTTPBuffer m_buffer;
    bdBool m_chunked;
    bdBool m_verboseOutput;
    bdBool m_progressMeter;
    bdUInt m_httpResponseCode;
    bdUInt64 m_expectedContentSize;
    bdInt64 m_socketErrorCode;
    bdFloat32 m_dataRate;
    bdUInt64 m_bytesTransfered;
    bdBool m_abort;
};