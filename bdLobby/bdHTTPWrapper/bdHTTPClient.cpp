// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

const bdNChar8* contentLengthHeader = "Content-Length: ";
const bdNChar8* headerEndStr = "\r\n\r\n";

void bdHTTPClient::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

void* bdHTTPClient::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

bdHTTPClient::bdHTTPClient()
    : m_port(80), m_downloadHandler(NULL), m_uploadHandler(NULL), m_operation(BD_HTTP_GET_OPERATION), m_status(BD_HTTP_STATUS_READY), m_chunked(false),
    m_verboseOutput(false), m_progressMeter(false), m_httpResponseCode(0), m_expectedContentSize(0), m_socketErrorCode(0), m_dataRate(0.0f),
    m_bytesTransfered(0), m_abort(false)
{
    m_serverName[0] = 0;
}

bdHTTPClient::~bdHTTPClient()
{
    if (m_status == BD_HTTP_STATUS_BUSY)
    {
        m_abort = true;
    }
}

bdBool bdHTTPClient::performOperation(bdStreamSocket* socket)
{
    bdStopwatch stopwatch;
    stopwatch.start();
    m_dataRate = 0.0f;
    m_bytesTransfered = 0;
    bdInt sendResult = socket->send(m_buffer.m_httpCommonBuffer, m_buffer.m_httpCommonBufferSize);
    if (sendResult < 0)
    {
        bdLogError("http", "Failed to send HTTP request with status code %d", sendResult);
        return false;
    }
    if ((m_buffer.m_contentSendBufferSize || m_uploadHandler) && !sendPayload(socket, &stopwatch))
    {
        return false;
    }

    bdUInt headerContentSize = 0;
    return recvResponseHeader(socket, &headerContentSize) && (m_operation || recvResponsePayload(socket, headerContentSize));
}

void bdHTTPClient::performOperation()
{
    if (m_abort)
    {
        bdLogError("http", "Not executing HTTP operation");
        return;
    }
    m_socketErrorCode = 1;
    m_status = BD_HTTP_STATUS_BUSY;
    bdAddr addr;
    bdStreamSocket sock;
    if (resolveHostIP(&addr) && !m_abort && sock.create(false))
    {
        bdSocketStatusCode sockStatus = sock.connect(&bdAddr(&addr));
        bdBool connected = false;
        bdBool canWrite;
        if (sockStatus == BD_NET_SUCCESS || sockStatus == BD_NET_WOULD_BLOCK)
        {
            for (canWrite = sock.isWritable(&sockStatus); !m_abort && !canWrite && sockStatus == BD_NET_SUCCESS; canWrite = sock.isWritable(&sockStatus))
            {
                bdPlatformTiming::sleep(50);
            }
            if (canWrite && sockStatus == BD_NET_SUCCESS)
            {
                connected = true;
                if (performOperation(&sock))
                {
                    m_socketErrorCode = 0;
                    m_status = BD_HTTP_STATUS_DONE;
                }
            }
        }
        if (!connected)
        {
            bdLogError("http", "Failed t oconnect to host");
            m_socketErrorCode = sockStatus;
        }
        sock.close();
    }
    if (m_status != BD_HTTP_STATUS_DONE)
    {
        m_status = BD_HTTP_STATUS_ERROR;
    }
}

bdBool bdHTTPClient::httpGet(bdNChar8* const serverName, bdNChar8* const downloadURL, void* const getBuffer, bdUInt bufferSize, bdDownloadInterceptor* downloadInterceptor, bdUInt port,
    bdUInt64 transactionID, bdUInt startByte, bdUInt endByte)
{
    if (!validateRequestInput(serverName, downloadURL, getBuffer, bufferSize, downloadInterceptor != 0))
    {
        m_status = BD_HTTP_STATUS_ERROR;
        return false;
    }
    clearHttpOperation();
    m_operation = BD_HTTP_GET_OPERATION;
    m_buffer.m_contentReceiveBuffer = getBuffer;
    m_buffer.m_contentReceiveBufferSize = bufferSize;
    if (endByte)
    {
        m_buffer.m_httpCommonBufferSize = bdSnprintf(
            m_buffer.m_httpCommonBuffer,
            1024,
            "GET %s HTTP/1.1\r\nHost: %s\r\nRange: bytes=%I32u-%I32u\r\nX-TransactionID: %I64u\r\n\r\n",
            downloadURL,
            serverName,
            startByte,
            endByte,
            transactionID);
    }
    else if (startByte)
    {
        m_buffer.m_httpCommonBufferSize = bdSnprintf(
            m_buffer.m_httpCommonBuffer,
            1024,
            "GET %s HTTP/1.1\r\nHost: %s\r\nRange: bytes=%I32u-\r\nX-TransactionID: %I64u\r\n\r\n",
            downloadURL,
            serverName,
            startByte,
            transactionID);
    }
    else
    {
        m_buffer.m_httpCommonBufferSize = bdSnprintf(
            m_buffer.m_httpCommonBuffer,
            1024,
            "GET %s HTTP/1.1\r\nHost: %s\r\nX-TransactionID: %I64u\r\n\r\n",
            downloadURL,
            serverName,
            transactionID);
    }
    if (m_buffer.m_httpCommonBufferSize > 0 && m_buffer.m_httpCommonBufferSize < 1023)
    {
        m_downloadHandler = downloadInterceptor;
        return initHttpOperation(serverName, port);
    }
    bdLogError("http", "Error writing http GET request string. Bytes written: %d.", m_buffer.m_httpCommonBufferSize);
    m_status = BD_HTTP_STATUS_ERROR;
    return false;
}

bdBool bdHTTPClient::httpPut(bdNChar8* const serverName, bdNChar8* const uploadURL, const void* uploadData, bdInt uploadSize, bdUploadInterceptor* uploadInterceptor, bdUInt port,
    bdUInt64 transactionID, bdNChar8* const checksum)
{
    if (!validateRequestInput(serverName, uploadURL, uploadData, uploadSize, uploadInterceptor != 0))
    {
        m_status = BD_HTTP_STATUS_ERROR;
        return 0;
    }
    clearHttpOperation();
    m_operation = BD_HTTP_PUT_OPERATION;
    m_buffer.m_contentSendBuffer = uploadData;
    m_buffer.m_contentSendBufferSize = uploadSize;
    if (uploadSize)
    {
        if (checksum)
        {
            m_buffer.m_httpCommonBufferSize = bdSnprintf(
                m_buffer.m_httpCommonBuffer,
                0x400u,
                "PUT %s HTTP/1.1\r\n"
                "Host: %s:%d\r\n"
                "Accept: */*\r\n"
                "Content-Length: %d\r\n"
                "X-Checksum: %s\r\n"
                "X-TransactionID: %I64u\r\n"
                "\r\n",
                uploadURL,
                serverName,
                port,
                uploadSize,
                checksum,
                transactionID);
        }
        else
        {
            m_buffer.m_httpCommonBufferSize = bdSnprintf(
                m_buffer.m_httpCommonBuffer,
                0x400u,
                "PUT %s HTTP/1.1\r\n"
                "Host: %s:%d\r\n"
                "Accept: */*\r\n"
                "Content-Length: %d\r\n"
                "X-TransactionID: %I64u\r\n"
                "\r\n",
                uploadURL,
                serverName,
                port,
                uploadSize,
                transactionID);
        }
    }
    else
    {
        m_buffer.m_httpCommonBufferSize = bdSnprintf(
            m_buffer.m_httpCommonBuffer,
            0x400u,
            "PUT %s HTTP/1.1\r\n"
            "Host: %s:%d\r\n"
            "Accept: */*\r\n"
            "Transfer-Encoding: chunked\r\n"
            "X-TransactionID: %I64u\r\n"
            "\r\n",
            uploadURL,
            serverName,
            port,
            transactionID);
    }
    if (m_buffer.m_httpCommonBufferSize > 0 && m_buffer.m_httpCommonBufferSize < 1023)
    {
        m_uploadHandler = uploadInterceptor;
        return initHttpOperation(serverName, port);
    }
    bdLogError("http", "Error writing http PUT request string. Bytes written: %d.", m_buffer.m_httpCommonBufferSize);
    m_status = BD_HTTP_STATUS_ERROR;
    return false;
}

bdBool bdHTTPClient::httpDelete(bdNChar8* const serverName, bdNChar8* const deleteURL, bdUInt port, bdUInt64 transactionID)
{
    if (!validateRequestInput(serverName, deleteURL, NULL, 0, 0))
    {
        m_status = BD_HTTP_STATUS_ERROR;
        return false;
    }
    clearHttpOperation();
    m_operation = BD_HTTP_DELETE_OPERATION;
    m_buffer.m_httpCommonBufferSize = bdSnprintf(
        m_buffer.m_httpCommonBuffer,
        1024,
        "DELETE %s HTTP/1.1\r\nHost: %s:%d\r\nX-TransactionID: %I64u\r\n\r\n",
        deleteURL,
        serverName,
        port,
        transactionID);
    if (m_buffer.m_httpCommonBufferSize <= 0)
    {
        bdLogError("http", "Error writing http request string");
        m_status = BD_HTTP_STATUS_ERROR;
        return false;
    }
    return initHttpOperation(serverName, port);
}

bdBool bdHTTPClient::httpCopy(bdNChar8* const serverName, bdNChar8* const copyURL, const bdNChar8* destination, bdUInt port, bdUInt64 transactionID)
{
    if (!validateRequestInput(serverName, copyURL, NULL, 0, false))
    {
        m_status = BD_HTTP_STATUS_ERROR;
        return false;
    }
    clearHttpOperation();
    m_operation = BD_HTTP_COPY_OPERATION;
    m_buffer.m_httpCommonBufferSize = bdSnprintf(
        m_buffer.m_httpCommonBuffer,
        1024,
        "COPY %s HTTP/1.1\r\nHost: %s:%d\r\nDestination: %s\r\nOverwrite: T\r\nX-TransactionID: %I64u\r\n\r\n",
        copyURL,
        serverName,
        port,
        destination,
        transactionID);
    if (m_buffer.m_httpCommonBufferSize <= 0)
    {
        bdLogError("http", "Error writing http request string");
        m_status = BD_HTTP_STATUS_ERROR;
        return false;
    }
    return initHttpOperation(serverName, port);
}

void bdHTTPClient::enableVerboseOutput(bdBool enable)
{
    m_verboseOutput = enable;
}

void bdHTTPClient::enableProgressMeter(bdBool enable)
{
    m_progressMeter = enable;
}

bdUInt64 bdHTTPClient::getResponseContentDataLength()
{
    return bdUInt64();
}

void bdHTTPClient::setResponseContentDataLength(bdUInt64)
{
}

bdHTTPClient::BD_HTTP_STATUS bdHTTPClient::getStatus()
{
    return m_status;
}

void bdHTTPClient::setStatus(bdHTTPClient::BD_HTTP_STATUS)
{
}

bdFloat32 bdHTTPClient::getDataRate()
{
    return m_dataRate;
}

void bdHTTPClient::setDataRate(bdFloat32 dataRate)
{
    m_dataRate = dataRate;
}

bdUInt64 bdHTTPClient::getResponseCode()
{
    return m_httpResponseCode;
}

bdInt64 bdHTTPClient::getSocketErrorCode()
{
    return m_socketErrorCode;
}

void bdHTTPClient::setSocketErrorCode(bdInt64)
{
}

bdHTTPBuffer* bdHTTPClient::getInternalBuffer()
{
    return &m_buffer;
}

bdHTTPClient::BD_HTTP_OPERATION bdHTTPClient::getOperation()
{
    return m_operation;
}

bdUInt64 bdHTTPClient::getExpectedContentSize()
{
    return m_expectedContentSize;
}

void bdHTTPClient::setBytesTransfered(bdUInt64 bytes)
{
    m_bytesTransfered = bytes;
}

bdUInt64 bdHTTPClient::getBytesTransfered()
{
    return m_bytesTransfered;
}

void bdHTTPClient::abortOperation()
{
    m_abort = true;
}

bdBool bdHTTPClient::isAborted()
{
    return m_abort;
}

bdInt bdHTTPClient::getSocketSendBuffSize()
{
    return bdInt();
}

void bdHTTPClient::setSocketSendBuffSize(bdInt)
{
}

bdBool bdHTTPClient::parseIncomingHttpResponse(bdNChar8* const responseBuffer, bdUInt bufferSize)
{
    if (!parseStatusLine(responseBuffer, bufferSize))
    {
        return false;
    }
    if (m_operation == BD_HTTP_GET_OPERATION)
    {
        bdNChar8* contentSize = bdStrstr(responseBuffer, contentLengthHeader);
        if (contentSize)
        {
            m_expectedContentSize = atoi(&contentSize[bdStrlen(contentLengthHeader)]);
        }
        else
        {
            bdLogError("http", "Error reading HTTP response: no Content-Length header found.");
        }
    }
    return true;
}

bdBool bdHTTPClient::parseStatusLine(bdNChar8* httpBuffer, bdUInt bufferSize)
{
    if (bufferSize < 12)
    {
        bdLogError("http", "Error parsing HTTP response: server response too short to be valid");
        return false;
    }
    if (bdMemcmp("HTTP", httpBuffer, 4u))
    {
        bdLogError("http", "Error parsing HTTP response: HTTP protocol string not found.");
        return false;
    }
    while (!isspace(*httpBuffer) && bufferSize)
    {
        --bufferSize;
        ++httpBuffer;
    }
    if (!bufferSize)
    {
        bdLogError("http", "Error parsing HTTP response: No space character found after HTTP protocol string.");
        return false;
    }
    m_httpResponseCode = atoi(httpBuffer);
    return m_httpResponseCode >= 200 && m_httpResponseCode < 300;
}

void bdHTTPClient::clearHttpOperation()
{
    m_serverName[0] = 0;
    m_port = 0;
    m_uploadHandler = 0;
    m_downloadHandler = 0;
    m_status = BD_HTTP_STATUS_READY;
    m_buffer.m_httpCommonBufferSize = 0;
    m_buffer.m_contentSendBuffer = 0;
    m_buffer.m_contentSendBufferSize = 0;
    m_buffer.m_contentReceiveBuffer = 0;
    m_buffer.m_contentReceiveBufferSize = 0;
    m_buffer.m_responseStatusBufferSize = 0;
    m_chunked = 0;
    m_httpResponseCode = 0;
    m_expectedContentSize = 0;
    m_socketErrorCode = 0;
    m_dataRate = 0.0f;
    m_bytesTransfered = 0;
    m_abort = 0;
}

bdBool bdHTTPClient::initHttpOperation(const bdNChar8* serverName, bdUInt port)
{
    m_status = BD_HTTP_STATUS_READY;
    if (bdSnprintf(m_serverName, sizeof(m_serverName), serverName) > 0)
    {
        m_port = port;
        return true;
    }
    bdLogError("http", "Error writing http server name '%s'", serverName);
    m_status = BD_HTTP_STATUS_ERROR;
    return false;
}

bdBool bdHTTPClient::validateRequestInput(const bdNChar8* serverName, const bdNChar8* requestPath, const void* inputBuffer, bdInt64 bufferSize, bdBool validInterceptor)
{
    if (m_status == BD_HTTP_STATUS_BUSY)
    {
        bdLogError("http", "Only one http operation may be processed at a time.");
        return false;
    }
    else if (serverName && requestPath && (bufferSize <= 0 || inputBuffer || validInterceptor))
    {
        bdUInt len = bdStrlen(serverName);
        if (len < 128 && len)
        {
            return true;
        }
        bdLogError("http", "HTTP server '%s' is invalid (perhaps too long?)", serverName);
        return false;
    }
    bdLogError("http", "HTTP input parameters invalid");
    return false;
}

bdBool bdHTTPClient::resolveHostIP(bdAddr* hostAddr)
{
    bdUInt i;

    bdInetAddr addr;
    addr.set(m_serverName);
    if (!addr.isValid() || addr.getInAddr().inUn.m_iaddr == -1)
    {
        bdGetHostByName getHostByName;
        getHostByName.start(m_serverName, bdGetHostByNameConfig(5.0f));
        getHostByName.pump();
        for (i = getHostByName.getStatus(); i == bdGetHostByName::BD_LOOKUP_SUCCEEDED; i = getHostByName.getStatus())
        {
            bdPlatformTiming::sleep(50);
            getHostByName.pump();
        }
        if (i == 2 && getHostByName.getNumAddresses())
        {
            addr.set(getHostByName.getAddressAt(0)->inUn.m_iaddr);
        }
    }

    if (!addr.isValid() || addr.getInAddr().inUn.m_iaddr == -1)
    {
        return false;
    }
    hostAddr->set(&addr, m_port);
    return true;
}

bdBool bdHTTPClient::sendPayload(bdStreamSocket* sock, bdStopwatch* httpTime)
{
    bdFloat32 elapsed = 0.0f;
    bdInt errorCode = 0;
    bdInt recvStatus = 0;
    bdUInt bytesSent = 0;
    bdBool ok = true;

    for (;;)
    {
        if (m_abort)
        {
            ok = false;
            break;
        }
        errorCode = writePayloadData(sock, &bytesSent);
        if (errorCode > 0)
        {
            setBytesTransfered(bytesSent);
            elapsed = httpTime->getElapsedTimeInSeconds();
            if (elapsed > 0.0f)
            {
                m_dataRate = bytesSent / elapsed;
            }
        }
        if (!m_httpResponseCode)
        {
            recvStatus = sock->recv(&m_buffer.m_responseStatusBuffer[m_buffer.m_responseStatusBufferSize], 128 - m_buffer.m_responseStatusBufferSize);
            if (recvStatus <= 0)
            {
                if (recvStatus != 2)
                {
                    errorCode = recvStatus;
                }
                if (errorCode <= 0)
                {
                    break;
                }
            }
            m_buffer.m_responseStatusBufferSize += recvStatus;
            if (m_buffer.m_responseStatusBufferSize >= 12 && !parseStatusLine(m_buffer.m_responseStatusBuffer, m_buffer.m_responseStatusBufferSize))
            {
                ok = false;
                break;
            }
        }
        if (errorCode <= 0)
        {
            break;
        }
    }
    if (errorCode < 0)
    {
        m_socketErrorCode = errorCode;
        return false;
    }
    return ok;
}

bdInt bdHTTPClient::writePayloadData(bdStreamSocket* sock, bdUInt* totalSent)
{
    const bdNChar8* payload;
    bdUInt totalSize;
    bdInt sentSize;
    bdInt j;

    bdUInt chunkSize = 0;
    if (m_uploadHandler)
    {
        bdUInt chunkLen = m_chunked ? 10 : 0;
        bdUInt chunkBytes = m_chunked ? 2 : 0;
        chunkSize = chunkBytes + chunkLen;
        bdUInt bytesToSend = m_uploadHandler->handleUpload(&m_buffer.m_httpCommonBuffer[chunkLen], 1024 - (chunkBytes + chunkLen), *totalSent);
        if (bytesToSend > 1024 - (chunkBytes + chunkLen))
        {
            return -1;
        }
        if (m_chunked)
        {
            bdInt bytesToSendLen = bdSnprintf(m_buffer.m_httpCommonBuffer, chunkLen, "%x", bytesToSend);
            m_buffer.m_httpCommonBuffer[chunkLen - 1] = '\n';
            m_buffer.m_httpCommonBuffer[chunkLen - 2] = '\r';
            for (; bytesToSendLen < chunkLen - 2; ++bytesToSendLen)
            {
                m_buffer.m_httpCommonBuffer[bytesToSendLen] = ' ';
            }
            m_buffer.m_httpCommonBuffer[chunkLen + bytesToSend] = '\r';
            m_buffer.m_httpCommonBuffer[chunkLen + bytesToSend + 1] = '\n';
        }
        payload = m_buffer.m_httpCommonBuffer;
        totalSize = chunkSize + bytesToSend;
    }
    else
    {
        payload = reinterpret_cast<const bdNChar8*>(m_buffer.m_contentSendBuffer) + *totalSent;
        totalSize = m_buffer.m_contentSendBufferSize - *totalSent >= 1024 ? 1024 : m_buffer.m_contentSendBufferSize - *totalSent;
    }

    do
    {
        for (j = sock->send(payload, totalSize); j == -2 && !m_abort; j = sock->send(payload, totalSize))
        {
            bdPlatformTiming::sleep(50);
        }
        if (j <= 0)
        {
            sentSize = j;
        }
        else
        {
            sentSize += j;
            totalSize -= j;
            payload += j;
        }
    } while (totalSize && j > 0);

    if (sentSize > 0)
    {
        if (chunkSize)
        {
            sentSize = sentSize <= chunkSize ? 0 : sentSize - chunkSize;
        }
        *totalSent += sentSize;
    }
    return sentSize;
}

bdBool bdHTTPClient::recvResponseHeader(bdStreamSocket* sock, bdUInt* bytesDownloaded)
{
    bdInt endBufferSize = -1;
    bdUInt oldSize = 0;
    bdNChar8* recvData = NULL;
    bdNChar8* responseBuffer = m_buffer.m_httpCommonBuffer;
    bdUInt headerMaxSize = 1024;
    if (m_buffer.m_responseStatusBufferSize)
    {
        bdMemcpy(responseBuffer, m_buffer.m_responseStatusBuffer, m_buffer.m_responseStatusBufferSize);
        oldSize = m_buffer.m_responseStatusBufferSize;
        m_buffer.m_responseStatusBufferSize = 0;
    }
    bdSocketStatusCode validHeaderEnd = readUntilStr(sock, headerEndStr, responseBuffer, headerMaxSize, &oldSize, &endBufferSize);
    if (endBufferSize < 0 && validHeaderEnd == BD_NET_SUCCESS)
    {
        recvData = bdAllocate<bdNChar8>(4096);
        bdMemcpy(recvData, responseBuffer, oldSize);
        headerMaxSize = 4096;
        responseBuffer = recvData;
        validHeaderEnd = readUntilStr(sock, headerEndStr, recvData, headerMaxSize, &oldSize, &endBufferSize);
    }
    *bytesDownloaded = 0;
    if (oldSize && validHeaderEnd == BD_NET_SUCCESS && endBufferSize >= 0 && parseIncomingHttpResponse(responseBuffer, oldSize))
    {
        endBufferSize += bdStrlen(headerEndStr);
        bdUInt contentSize = oldSize - endBufferSize;
        bdBool ok = true;
        if (m_downloadHandler)
        {
            if (contentSize && !m_downloadHandler->handleDownload(&responseBuffer[endBufferSize], contentSize))
            {
                ok = false;
            }
            *bytesDownloaded = contentSize;
        }
        else if (contentSize <= m_buffer.m_contentReceiveBufferSize)
        {
            if (contentSize)
            {
                memmove(m_buffer.m_contentReceiveBuffer, &responseBuffer[endBufferSize], contentSize);
                *bytesDownloaded = contentSize;
            }
        }
        else
        {
            ok = false;
        }
        if (recvData)
        {
            bdDeallocate<bdNChar8>(recvData);
        }
        return ok;
    }
    if (recvData)
    {
        bdDeallocate<bdNChar8>(recvData);
    }
    bdLogError("http", "Error occured when reading response from server.");
    return false;
}

bdBool bdHTTPClient::recvResponsePayload(bdStreamSocket* sock, bdUInt bytesDownloaded)
{
    bdStopwatch stopwatch;
    stopwatch.start();
    bdFloat32 elapsed = 0.0f;
    bdBool isError = false;
    bdUInt recvSize = 0;
    
    while (bytesDownloaded < m_expectedContentSize && !isError && !m_abort)
    {
        recvSize = sock->recv(m_buffer.m_httpCommonBuffer, 1024);
        if (recvSize == -2)
        {
            bdPlatformTiming::sleep(50);
            continue;
        }
        if (recvSize <= 0)
        {
            break;
        }
        if (m_downloadHandler)
        {
            if (!m_downloadHandler->handleDownload(m_buffer.m_httpCommonBuffer, recvSize))
            {
                isError = true;
            }
        }
        else
        {
            if (recvSize + bytesDownloaded > m_buffer.m_contentReceiveBufferSize)
            {
                return false;
            }
            bdMemcpy(reinterpret_cast<bdNChar8*>(m_buffer.m_contentReceiveBuffer) + bytesDownloaded, m_buffer.m_httpCommonBuffer, recvSize);
        }
        bytesDownloaded += recvSize;
        elapsed = stopwatch.getElapsedTimeInSeconds();
        setBytesTransfered(bytesDownloaded);
        if (elapsed > 0.0)
        {
            setDataRate(bytesDownloaded / elapsed);
        }
    }
    if (recvSize >= -0)
    {
        elapsed = stopwatch.getElapsedTimeInSeconds();
        setBytesTransfered(bytesDownloaded);
        if (elapsed > 0.0)
        {
            setDataRate(bytesDownloaded / elapsed);
        }

        if (bytesDownloaded >= m_expectedContentSize)
        {
            return true;
        }
        bdLogError("http", "Expected to receive %u bytes but instead received %u bytes", m_expectedContentSize, bytesDownloaded);
        return false;
    }
    bdLogError("http", "Error %d occured when reading http payload", recvSize);
    return false;
}

bdSocketStatusCode bdHTTPClient::readUntilStr(bdStreamSocket* socket, const bdNChar8* endString, bdNChar8* recvData, bdUInt maxSize, bdUInt* bufSize, bdInt* endSize)
{
    bdNChar8* endLoc;
    bdInt recvd;

    bdUInt remainingSize = maxSize > *bufSize ? maxSize - *bufSize - 1 : 0;
    bdNChar8* recvStart = &recvData[*bufSize];
    *endSize = -1;
    if (*bufSize && remainingSize && (*recvStart = 0, (endLoc = strstr(recvData, endString)) != 0))
    {
        *endSize = endLoc - recvData;
        return BD_NET_SUCCESS;
    }
    while (remainingSize)
    {
        recvd = socket->recv(recvStart, remainingSize);
        if (m_abort || recvd != -2)
        {
            if (m_abort || recvd <= 0)
            {
                return BD_NET_ERROR;
            }
            remainingSize -= recvd;
            *bufSize += recvd;
            recvStart += recvd;
            *recvStart = 0;
            if (endLoc = bdStrstr(recvData, endString))
            {
                *endSize = endLoc - recvData;
                return BD_NET_SUCCESS;
            }
        }
        else
        {
            bdPlatformTiming::sleep(50);
        }
    }
    return BD_NET_SUCCESS;
}
