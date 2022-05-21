// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdGroupCount : public bdTaskResult
{
public:
    bdUInt32 m_groupID;
    bdUInt32 m_groupCount;

    virtual bdBool deserialize(bdByteBufferRef buffer);
    virtual void serialize(bdByteBuffer* buffer);
    virtual bdUInt sizeOf();
};