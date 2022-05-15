// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdFileID : public bdTaskResult
{
public:
    bdUInt64 m_fileID;

    bdFileID();
    ~bdFileID();
    virtual void serialize(bdByteBuffer* buffer);
    virtual bdBool deserialize(bdByteBufferRef buffer);
    virtual bdUInt sizeOf();
};