// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdHTTPWrapper : public bdHTTPWrapperBase
{
protected:
    bdHTTPClient m_httpClient;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdHTTPWrapper();
    ~bdHTTPWrapper();
    bdHTTPWrapperBase::bdStatus startDownload(const bdNChar8* const downloadURL, bdDownloadInterceptor* downloadHandler, bdUInt64 transactionID, bdUInt startByte, bdUInt endByte);
    bdHTTPWrapperBase::bdStatus startDownload(const bdNChar8* const downloadURL, void* const downloadBuffer, bdUInt bufferSize, bdUInt64 transactionID, bdUInt startByte, bdUInt endByte);
    bdHTTPWrapperBase::bdStatus startUpload(const bdNChar8* const uploadURL, bdUploadInterceptor* uploadHandler, bdUInt uploadSize, bdUInt64 transactionID);
    bdHTTPWrapperBase::bdStatus startUpload(const bdNChar8* const uploadURL, const void* const uploadBuffer, bdUInt uploadSize, bdUInt64 transactionID, bdNChar8* checkSum);
    bdHTTPWrapperBase::bdStatus startCopy(const bdNChar8* const copyURL, const bdNChar8* destination, bdUInt64 transactionID);
    bdHTTPWrapperBase::bdStatus startDelete(const bdNChar8* const deleteURL, bdUInt64 transactionID);
    void enableVerboseOutput(bdBool enable);
    void enableProgressMeter(bdBool enable);
    void abortOperation();
    bdBool abortInProgress();
    bdUInt getTransferProgress();
    bdFloat32 getTransferSpeed();
    void resetTransferStats();
    bdInt getLastHTTPError();
    bdUInt run(void* args);
    bdHTTPWrapperBase::bdStatus _startDownload(const bdNChar8* const downloadURL, void* const downloadBuffer, bdUInt bufferSize, bdDownloadInterceptor* downloadHandler,
        bdUInt64 transactionID, bdUInt startByte, bdUInt endByte);
    bdHTTPWrapperBase::bdStatus _startUpload(const bdNChar8* const uploadURL, const void* const fileData, bdUploadInterceptor* uploadHandler, bdUInt uploadSize,
        bdUInt64 transactionID, bdNChar8* checkSum);
};