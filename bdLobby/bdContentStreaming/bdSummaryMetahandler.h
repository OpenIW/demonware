// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdSummaryMetaHandler : public bdFileMetaData
{
public:
    void operator delete(void* p);
    bdSummaryMetaHandler();
    ~bdSummaryMetaHandler();
    virtual bdBool deserialize(bdByteBufferRef buffer);
    virtual bdUInt sizeOf();
};